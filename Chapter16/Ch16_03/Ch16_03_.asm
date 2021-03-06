;-------------------------------------------------
;               Ch16_03_.asm
;-------------------------------------------------

        include <Ch16_03_.asmh>

; Macro _LlTraverse
;
; The following macro generates linked list traversal code using the
; prefetchnta instruction if UsePrefetch is equal to 'Y'.

_LlTraverse macro UsePrefetch
        mov rax,rcx                                 ;rax = ptr to 1st node
        test rax,rax
        jz Done                                     ;jump if empty list

        align 16
@@::    mov rcx,[rax+LlNode.Link]                   ;rcx = next node
        vmovapd ymm0,ymmword ptr [rax+LlNode.ValA]  ;ymm0 = ValA
        vmovapd ymm1,ymmword ptr [rax+LLNode.ValB]  ;ymm1 = ValB

IFIDNI <UsePrefetch>,<Y>
        mov rdx,rcx
        test rdx,rdx                        ;is there another node?
        cmovz rdx,rax                       ;avoid prefetch of nullptr
        prefetchnta [rdx]                   ;prefetch start of next node
ENDIF

; Calculate ValC[i] = sqrt(ValA[i] * ValA[i] + ValB[i] * ValB[i])
        vmulpd ymm2,ymm0,ymm0               ;ymm2 = ValA * ValA
        vmulpd ymm3,ymm1,ymm1               ;ymm3 = ValB * ValB
        vaddpd ymm4,ymm2,ymm3               ;ymm4 = sums
        vsqrtpd ymm5,ymm4                   ;ymm5 = square roots

        vmovntpd ymmword ptr [rax+LlNode.ValC],ymm5 ;save result

; Calculate ValD[i] = sqrt(ValA[i] / ValB[i] + ValB[i] / ValA[i]);
        vdivpd ymm2,ymm0,ymm1               ;ymm2 = ValA / ValB
        vdivpd ymm3,ymm1,ymm0               ;ymm3 = ValB / ValA
        vaddpd ymm4,ymm2,ymm3               ;ymm4 = sums
        vsqrtpd ymm5,ymm4                   ;ymm5 = square roots

        vmovntpd ymmword ptr [rax+LlNode.ValD],ymm5 ;save result

        mov rax,rcx                         ;rax = ptr to next node
        test rax,rax
        jnz @B

Done:   vzeroupper
        ret
        endm

; extern "C" void LlTraverseA_(LlNode* first);

        .code
LlTraverseA_ proc
        _LlTraverse n
LlTraverseA_ endp

; extern "C" void LlTraverseB_(LlNode* first);

LlTraverseB_ proc
        _LlTraverse y
LlTraverseB_ endp
        end