//------------------------------------------------
//               Ch13_07.cpp
//------------------------------------------------

#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <memory>
#include "Ch13_07.h"
#include "AlignedMem.h"

using namespace std;

void InitVec(Vector* a_aos, Vector* b_aos, VectorSoA& a_soa, VectorSoA& b_soa, size_t num_vec)
{
    uniform_int_distribution<> ui_dist {1, 100};
    default_random_engine rng {103};

    for (size_t i = 0; i < num_vec; i++)
    {
        double a_x = (double)ui_dist(rng);
        double a_y = (double)ui_dist(rng);
        double a_z = (double)ui_dist(rng);
        double b_x = (double)ui_dist(rng);
        double b_y = (double)ui_dist(rng);
        double b_z = (double)ui_dist(rng);

        a_aos[i].X = a_soa.X[i] = a_x;
        a_aos[i].Y = a_soa.Y[i] = a_y;
        a_aos[i].Z = a_soa.Z[i] = a_z;

        b_aos[i].X = b_soa.X[i] = b_x;
        b_aos[i].Y = b_soa.Y[i] = b_y;
        b_aos[i].Z = b_soa.Z[i] = b_z;
    }
}

void Avx512Vcp(void)
{
    const size_t align = 64;
    const size_t num_vec = 16;

    unique_ptr<Vector> a_aos_up {new Vector[num_vec] };
    unique_ptr<Vector> b_aos_up {new Vector[num_vec] };
    unique_ptr<Vector> c_aos_up {new Vector[num_vec] };
    Vector* a_aos = a_aos_up.get();
    Vector* b_aos = b_aos_up.get();
    Vector* c_aos = c_aos_up.get();

    VectorSoA a_soa, b_soa, c_soa;
    AlignedArray<double> a_soa_x_aa(num_vec, align);
    AlignedArray<double> a_soa_y_aa(num_vec, align);
    AlignedArray<double> a_soa_z_aa(num_vec, align);
    AlignedArray<double> b_soa_x_aa(num_vec, align);
    AlignedArray<double> b_soa_y_aa(num_vec, align);
    AlignedArray<double> b_soa_z_aa(num_vec, align);
    AlignedArray<double> c_soa_x_aa(num_vec, align);
    AlignedArray<double> c_soa_y_aa(num_vec, align);
    AlignedArray<double> c_soa_z_aa(num_vec, align);
    a_soa.X = a_soa_x_aa.Data();
    a_soa.Y = a_soa_y_aa.Data();
    a_soa.Z = a_soa_z_aa.Data();
    b_soa.X = b_soa_x_aa.Data();
    b_soa.Y = b_soa_y_aa.Data();
    b_soa.Z = b_soa_z_aa.Data();
    c_soa.X = c_soa_x_aa.Data();
    c_soa.Y = c_soa_y_aa.Data();
    c_soa.Z = c_soa_z_aa.Data();

    InitVec(a_aos, b_aos, a_soa, b_soa, num_vec);

    bool rc1 = Avx512VcpAos_(c_aos, a_aos, b_aos, num_vec);
    bool rc2 = Avx512VcpSoa_(&c_soa, &a_soa, &b_soa, num_vec);

    cout << "Results for Avx512VectorCrossProd\n";

    if (!rc1 || !rc2)
    {
        cout << "Invalid return code - ";
        cout << "rc1 = " << boolalpha << rc1 << ", ";
        cout << "rc2 = " << boolalpha << rc2 << ", ";
        return;
     }

    cout << fixed << setprecision(1);

    for (size_t i = 0; i < num_vec; i++)
    {
        cout << "Vector cross product #" << i << '\n';

        const unsigned int w = 9;

        cout << "  a:      ";
        cout << setw(w) << a_aos[i].X << ' ';
        cout << setw(w) << a_aos[i].Y << ' ';
        cout << setw(w) << a_aos[i].Z << '\n';

        cout << "  b:      ";
        cout << setw(w) << b_aos[i].X << ' ';
        cout << setw(w) << b_aos[i].Y << ' ';
        cout << setw(w) << b_aos[i].Z << '\n';

        cout << "  c_aos:  ";
        cout << setw(w) << c_aos[i].X << ' ';
        cout << setw(w) << c_aos[i].Y << ' ';
        cout << setw(w) << c_aos[i].Z << '\n';

        cout << "  c_soa:  ";
        cout << setw(w) << c_soa.X[i] << ' ';
        cout << setw(w) << c_soa.Y[i] << ' ';
        cout << setw(w) << c_soa.Z[i] << '\n';

        bool is_valid_x = c_aos[i].X == c_soa.X[i];
        bool is_valid_y = c_aos[i].Y == c_soa.Y[i];
        bool is_valid_z = c_aos[i].Z == c_soa.Z[i];

        if (!is_valid_x || !is_valid_y || !is_valid_z)
        {
            cout << "Compare error at index " << i << '\n';
            cout << "  is_valid_x = " << boolalpha << is_valid_x << '\n';
            cout << "  is_valid_y = " << boolalpha << is_valid_y << '\n';
            cout << "  is_valid_z = " << boolalpha << is_valid_z << '\n';
            return;
        }
    }
}

int main()
{
    Avx512Vcp();
    Avx512Vcp_BM();
    return 0;
}
