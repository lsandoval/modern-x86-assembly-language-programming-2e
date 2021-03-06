//------------------------------------------------
//               Ch10_06.cpp
//------------------------------------------------

#include "stdafx.h"
#include <iostream>
#include <stdexcept>
#include "Ch10_06.h"
#include "AlignedMem.h"
#include "ImageMatrix.h"

using namespace std;

// Image size limits
extern "C" const int c_NumPixelsMin = 32;
extern "C" const int c_NumPixelsMax = 256 * 1024 * 1024;

// RGB to grayscale conversion coefficients
const float c_Coef[4] {0.2126f, 0.7152f, 0.0722f, 0.0f};

bool CompareGsImages(const uint8_t* pb_gs1,const uint8_t* pb_gs2, int num_pixels)
{
    for (int i = 0; i < num_pixels; i++)
    {
        if (abs((int)pb_gs1[i] - (int)pb_gs2[i]) > 1)
            return false;
    }

    return true;
}

bool Avx2ConvertRgbToGsCpp(uint8_t* pb_gs, const RGB32* pb_rgb, int num_pixels, const float coef[4])
{
    if (num_pixels < c_NumPixelsMin || num_pixels > c_NumPixelsMax)
        return false;
    if (num_pixels % 8 != 0)
        return false;

    if (!AlignedMem::IsAligned(pb_gs, 32))
        return false;
    if (!AlignedMem::IsAligned(pb_rgb, 32))
        return false;

    for (int i = 0; i < num_pixels; i++)
    {
        uint8_t r = pb_rgb[i].m_R;
        uint8_t g = pb_rgb[i].m_G;
        uint8_t b = pb_rgb[i].m_B;

        float gs_temp = r * coef[0] + g * coef[1] + b * coef[2] + 0.5f;

        if (gs_temp < 0.0f)
            gs_temp = 0.0f;
        else if (gs_temp > 255.0f)
            gs_temp = 255.0f;

        pb_gs[i] = (uint8_t)gs_temp;
    }

    return true;
}

void Avx2ConvertRgbToGs(void)
{
    const wchar_t* fn_rgb = L"..\\Ch10_Data\\TestImage3.bmp";
    const wchar_t* fn_gs1 = L"Ch10_06_Avx2ConvertRgbToGs_TestImage3_GS1.bmp";
    const wchar_t* fn_gs2 = L"Ch10_06_Avx2ConvertRgbToGs_TestImage3_GS2.bmp";

    ImageMatrix im_rgb(fn_rgb);
    int im_h = im_rgb.GetHeight();
    int im_w = im_rgb.GetWidth();
    int num_pixels = im_h * im_w;
    ImageMatrix im_gs1(im_h, im_w, PixelType::Gray8);
    ImageMatrix im_gs2(im_h, im_w, PixelType::Gray8);
    RGB32* pb_rgb = im_rgb.GetPixelBuffer<RGB32>();
    uint8_t* pb_gs1 = im_gs1.GetPixelBuffer<uint8_t>();
    uint8_t* pb_gs2 = im_gs2.GetPixelBuffer<uint8_t>();

    cout << "Results for Avx2ConvertRgbToGs\n";
    wcout << "Converting RGB image " << fn_rgb << '\n';
    cout << "  im_h = " << im_h << " pixels\n";
    cout << "  im_w = " << im_w << " pixels\n";

    // Exercise conversion functions
    bool rc1 = Avx2ConvertRgbToGsCpp(pb_gs1, pb_rgb, num_pixels, c_Coef);
    bool rc2 = Avx2ConvertRgbToGs_(pb_gs2, pb_rgb, num_pixels, c_Coef);

    if (rc1 && rc2)
    {
        wcout << "Saving grayscale image #1 - " << fn_gs1 << '\n';
        im_gs1.SaveToBitmapFile(fn_gs1);

        wcout << "Saving grayscale image #2 - " << fn_gs2 << '\n';
        im_gs2.SaveToBitmapFile(fn_gs2);

        if (CompareGsImages(pb_gs1, pb_gs2, num_pixels))
            cout << "Grayscale image compare OK\n";
        else
            cout << "Grayscale image compare failed\n";
    }
    else
        cout << "Invalid return code\n";
}

int main()
{
    try
    {
        Avx2ConvertRgbToGs();
        Avx2ConvertRgbToGs_BM();
    }

    catch (runtime_error& rte)
    {
        cout << "'runtime_error' exception has occurred - " << rte.what() << '\n';
    }

    catch (...)
    {
        cout << "Unexpected exception has occurred\n";
    }

    return 0;
}
