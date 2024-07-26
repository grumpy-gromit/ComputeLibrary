/*
 * Copyright (c) 2022-2024 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#if defined(ARM_COMPUTE_ENABLE_SME2)

#include "arm_gemm.hpp"
#include "../../utils.hpp"
#include "../../bfloat.hpp"

#include <cassert>
#include <limits>

namespace arm_gemm {

void sme2_gemv_fp32bf16fp32_dot_16VL (
    const float *A_ptr, const bfloat16 *B_ptr, float *output_ptr,
    size_t N, size_t K,
    const float *bias, Activation act, bool
)
{
    struct KernelArgs {
        float maxval = static_cast<float>(std::numeric_limits<float>::infinity());
        float minval = - static_cast<float>(std::numeric_limits<float>::infinity());
        const bfloat16 *B_ptr = {};
        size_t output_offset = {};
        unsigned int input_initial_col = {};
    } ka;

    unsigned long flags=0;
    ka.B_ptr = B_ptr;
    switch(act.type) {
        default:
        case Activation::Type::None:
            break;
        case Activation::Type::BoundedReLU:
            ka.maxval = static_cast<float>(act.param1);
            /* fall through */
        case Activation::Type::ReLU:
            ka.minval = 0;
            flags |= 0x2;
            break;
    }
    __asm__ __volatile__(
      "ptrue p8.b\n"
      ".inst 0xd503477f  // SMSTART ZA\n"
      "cntw x10, ALL, MUL #4\n"
      "add x28, %x[N], x10\n"
      "sub x28, x28, #0x1\n"
      "udiv x28, x28, x10\n"
      "add x22, x28, #0x3\n"
      "and x22, x22, #0xfffffffffffffffc\n"
      "mul x22, x22, x10\n"
      "mul x22, x22, %x[K]\n"
      "mov x9, #0x0\n"
      "mov x27, #0x4\n"
      "mov x26, %x[B_ptr]\n"
      "mov x25, %x[output_ptr]\n"
      "ptrue p2.b\n"
      ".inst 0x25207811  // ptrue pn9.b\n"
      "lsl x22, x22, #0x1\n"
      "mov x21, #0x1\n"
      "1:"  // RHS size check loop
      "cmp x22, #0x200000\n"
      "blt 2f\n"
      "tbnz x22, #0, 3f\n"
      "lsr x22, x22, #0x1\n"
      "lsl x21, x21, #0x1\n"
      "b 1b\n"
      "2:"  // RHS do prefetch
      "lsl x20, x22, #0x26\n"
      "sub x21, x21, #0x1\n"
      "lsl x21, x21, #0x16\n"
      "orr x22, x22, x20\n"
      "orr x22, x22, x21\n"
      ".inst 0xf8b64b5a  // rprfm pldonce, x22, [x26]\n"
      "3:"  // RHS prefetch exit
      "mov x24, %x[bias]\n"
      "4:"  // Column loop
      "cmp x28, #0x4\n"
      "bge 28f\n"
      "cmp x28, #0x2\n"
      "bgt 20f\n"
      "beq 12f\n"
      "mov x23, %x[A_ptr]\n"
      "lsl x21, %x[K], #0x2\n"
      "mov x20, %x[N]\n"
      "mov x22, %x[K]\n"
      ".inst 0xf8b54af8  // rprfm pldmany, x21, [x23]\n"
      ".inst 0x25b467f0  // whilelt p8.s, XZR, x20, VLx4\n"
      "cbz x24, 5f\n"
      ".inst 0xa040c710  // ld1w { z16.s-z19.s }, pn9.b/Z, [x24]\n"
      ".inst 0xc0042e00  // mova za.d[x9, #0], { z16.d-z19.d }\n"
      "b 6f\n"
      "5:"  // Width 1: no bias
      ".inst 0xc00800ff  // zero { zad0, zad1, zad2, zad3, zad4, zad5, zad6, zad7 }\n"
      "6:"  // Width 1: setup done
      "cmp x22, #0x8\n"
      "ble 8f\n"
      "7:"  // Width 1: Multiply loop: Main loop head
      "whilelt p1.s, XZR, x22\n"
      "whilelt p0.s, x27, x22\n"
      "ld1rqw { z10.s }, p1/Z, [x23]\n"
      ".inst 0x658aa94a  // bfcvt z10.h, p2/M, z10.s\n"
      "ld1rqw { z16.s }, p0/Z, [x23, #16]\n"
      ".inst 0x658aaa10  // bfcvt z16.h, p2/M, z16.s\n"
      "uzp1 z10.h, z10.h, z10.h\n"
      "sub x22, x22, #0x8\n"
      "uzp1 z16.h, z16.h, z16.h\n"
      "trn1 z10.d, z10.d, z16.d\n"
      ".inst 0xa040a74d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x26]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15ab198  // bfdot za.s[x9, 0], { z12.h-z15.h }, z10.h[0]\n"
      ".inst 0xa040a74d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x26]\n"
      "addvl x26, x26, #16\n"
      "cmp x22, #0x8\n"
      ".inst 0xc15ab598  // bfdot za.s[x9, 0], { z12.h-z15.h }, z10.h[1]\n"
      ".inst 0xa040a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26]\n"
      "addvl x26, x26, #16\n"
      "add x23, x23, #0x20\n"
      ".inst 0xc15ab818  // bfdot za.s[x9, 0], { z0.h-z3.h }, z10.h[2]\n"
      ".inst 0xa040a75d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x26]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15abf98  // bfdot za.s[x9, 0], { z28.h-z31.h }, z10.h[3]\n"
      "bgt 7b\n"
      "8:"  // Width 1: Multiply loop: Single iteration only
      "whilelt p1.s, XZR, x22\n"
      "whilelt p0.s, x27, x22\n"
      "ld1rqw { z15.s }, p1/Z, [x23]\n"
      ".inst 0x658aa9ef  // bfcvt z15.h, p2/M, z15.s\n"
      "ld1rqw { z17.s }, p0/Z, [x23, #16]\n"
      ".inst 0x658aaa31  // bfcvt z17.h, p2/M, z17.s\n"
      "uzp1 z15.h, z15.h, z15.h\n"
      "subs x22, x22, #0x2\n"
      "uzp1 z17.h, z17.h, z17.h\n"
      "trn1 z15.d, z15.d, z17.d\n"
      ".inst 0xa040a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26]\n"
      "add x23, x23, #0x20\n"
      ".inst 0xc15fb218  // bfdot za.s[x9, 0], { z16.h-z19.h }, z15.h[0]\n"
      "addvl x26, x26, #16\n"
      "ble 9f\n"
      ".inst 0xa040a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xc15fb418  // bfdot za.s[x9, 0], { z0.h-z3.h }, z15.h[1]\n"
      "addvl x26, x26, #16\n"
      "ble 9f\n"
      ".inst 0xa040a745  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x26]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xc15fb898  // bfdot za.s[x9, 0], { z4.h-z7.h }, z15.h[2]\n"
      "addvl x26, x26, #16\n"
      "ble 9f\n"
      ".inst 0xa040a749  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x26]\n"
      ".inst 0xc15fbd18  // bfdot za.s[x9, 0], { z8.h-z11.h }, z15.h[3]\n"
      "addvl x26, x26, #16\n"
      "9:"  // Width 1: Multiply loop: multiply skip
      "tbz %x[flags], #1, 10f\n"
      "add x21, %x[args_ptr], %[offset_min]\n"
      "add x20, %x[args_ptr], %[offset_max]\n"
      ".inst 0xc0062c00  // mova { z0.d-z3.d }, za.d[x9, #0]\n"
      "ld1rw { z8.s }, p2/Z, [x21]\n"
      "ld1rw { z26.s }, p2/Z, [x20]\n"
      ".inst 0xc1bac900  // fclamp { z0.s-z3.s }, z8.s, z26.s\n"
      ".inst 0xa060c320  // st1w { z0.s-z3.s }, p8, [x25]\n"
      "addvl x25, x25, #4\n"
      "b 11f\n"
      "10:"  // Width 1: No activation
      ".inst 0xc0062c04  // mova { z4.d-z7.d }, za.d[x9, #0]\n"
      ".inst 0xa060c324  // st1w { z4.s-z7.s }, p8, [x25]\n"
      "addvl x25, x25, #4\n"
      "11:"  // Width 1: Output done
      "b 36f\n"
      "12:"  // Width 2
      "mov x23, %x[A_ptr]\n"
      "lsl x21, %x[K], #0x2\n"
      "sub x20, %x[N], x10\n"
      "mov x22, %x[K]\n"
      ".inst 0xf8b54af8  // rprfm pldmany, x21, [x23]\n"
      ".inst 0x25b467f0  // whilelt p8.s, XZR, x20, VLx4\n"
      "cbz x24, 13f\n"
      ".inst 0xa040c718  // ld1w { z24.s-z27.s }, pn9.b/Z, [x24]\n"
      ".inst 0xc0042f00  // mova za.d[x9, #0], { z24.d-z27.d }\n"
      ".inst 0xa041c710  // ld1w { z16.s-z19.s }, pn9.b/Z, [x24, #0x4, MUL VL]\n"
      ".inst 0xc0042e01  // mova za.d[x9, #1], { z16.d-z19.d }\n"
      "b 14f\n"
      "13:"  // Width 2: no bias
      ".inst 0xc00800ff  // zero { zad0, zad1, zad2, zad3, zad4, zad5, zad6, zad7 }\n"
      "14:"  // Width 2: setup done
      "cmp x22, #0x8\n"
      "ble 16f\n"
      "15:"  // Width 2: Multiply loop: Main loop head
      "whilelt p1.s, XZR, x22\n"
      "whilelt p0.s, x27, x22\n"
      "ld1rqw { z13.s }, p1/Z, [x23]\n"
      ".inst 0x658aa9ad  // bfcvt z13.h, p2/M, z13.s\n"
      "ld1rqw { z27.s }, p0/Z, [x23, #16]\n"
      ".inst 0x658aab7b  // bfcvt z27.h, p2/M, z27.s\n"
      "uzp1 z13.h, z13.h, z13.h\n"
      "sub x22, x22, #0x8\n"
      "uzp1 z27.h, z27.h, z27.h\n"
      "trn1 z13.d, z13.d, z27.d\n"
      ".inst 0xa040a755  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x26]\n"
      "cmp x22, #0x8\n"
      ".inst 0xa041a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15db298  // bfdot za.s[x9, 0], { z20.h-z23.h }, z13.h[0]\n"
      "addvl x26, x26, #16\n"
      "add x23, x23, #0x20\n"
      ".inst 0xc15db019  // bfdot za.s[x9, 1], { z0.h-z3.h }, z13.h[0]\n"
      ".inst 0xa040a755  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x26]\n"
      ".inst 0xa041a759  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15db698  // bfdot za.s[x9, 0], { z20.h-z23.h }, z13.h[1]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15db719  // bfdot za.s[x9, 1], { z24.h-z27.h }, z13.h[1]\n"
      ".inst 0xa040a749  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x26]\n"
      ".inst 0xa041a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15db918  // bfdot za.s[x9, 0], { z8.h-z11.h }, z13.h[2]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15dba19  // bfdot za.s[x9, 1], { z16.h-z19.h }, z13.h[2]\n"
      ".inst 0xa040a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26]\n"
      ".inst 0xa041a745  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15dbc18  // bfdot za.s[x9, 0], { z0.h-z3.h }, z13.h[3]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15dbc99  // bfdot za.s[x9, 1], { z4.h-z7.h }, z13.h[3]\n"
      "bgt 15b\n"
      "16:"  // Width 2: Multiply loop: Single iteration only
      "whilelt p1.s, XZR, x22\n"
      "whilelt p0.s, x27, x22\n"
      "ld1rqw { z15.s }, p1/Z, [x23]\n"
      ".inst 0x658aa9ef  // bfcvt z15.h, p2/M, z15.s\n"
      "ld1rqw { z5.s }, p0/Z, [x23, #16]\n"
      ".inst 0x658aa8a5  // bfcvt z5.h, p2/M, z5.s\n"
      "uzp1 z15.h, z15.h, z15.h\n"
      "subs x22, x22, #0x2\n"
      "uzp1 z5.h, z5.h, z5.h\n"
      "trn1 z15.d, z15.d, z5.d\n"
      ".inst 0xa040a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26]\n"
      "add x23, x23, #0x20\n"
      ".inst 0xa041a759  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fb218  // bfdot za.s[x9, 0], { z16.h-z19.h }, z15.h[0]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15fb319  // bfdot za.s[x9, 1], { z24.h-z27.h }, z15.h[0]\n"
      "ble 17f\n"
      ".inst 0xa040a75d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x26]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xc15fb798  // bfdot za.s[x9, 0], { z28.h-z31.h }, z15.h[1]\n"
      ".inst 0xa041a745  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fb499  // bfdot za.s[x9, 1], { z4.h-z7.h }, z15.h[1]\n"
      "addvl x26, x26, #16\n"
      "ble 17f\n"
      ".inst 0xa040a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xc15fba18  // bfdot za.s[x9, 0], { z16.h-z19.h }, z15.h[2]\n"
      ".inst 0xa041a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fba19  // bfdot za.s[x9, 1], { z16.h-z19.h }, z15.h[2]\n"
      "addvl x26, x26, #16\n"
      "ble 17f\n"
      ".inst 0xa040a75d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x26]\n"
      ".inst 0xc15fbf98  // bfdot za.s[x9, 0], { z28.h-z31.h }, z15.h[3]\n"
      ".inst 0xa041a749  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fbd19  // bfdot za.s[x9, 1], { z8.h-z11.h }, z15.h[3]\n"
      "addvl x26, x26, #16\n"
      "17:"  // Width 2: Multiply loop: multiply skip
      "tbz %x[flags], #1, 18f\n"
      "add x21, %x[args_ptr], %[offset_min]\n"
      "add x20, %x[args_ptr], %[offset_max]\n"
      ".inst 0xc0062c14  // mova { z20.d-z23.d }, za.d[x9, #0]\n"
      "ld1rw { z11.s }, p2/Z, [x21]\n"
      ".inst 0xc0062c2c  // mova { z12.d-z15.d }, za.d[x9, #1]\n"
      "ld1rw { z28.s }, p2/Z, [x20]\n"
      ".inst 0xc1bcc974  // fclamp { z20.s-z23.s }, z11.s, z28.s\n"
      ".inst 0xa060c734  // st1w { z20.s-z23.s }, pn9.b, [x25]\n"
      ".inst 0xc1bcc96c  // fclamp { z12.s-z15.s }, z11.s, z28.s\n"
      ".inst 0xa061c32c  // st1w { z12.s-z15.s }, p8, [x25, #0x4, MUL VL]\n"
      "addvl x25, x25, #8\n"
      "b 19f\n"
      "18:"  // Width 2: No activation
      ".inst 0xc0062c00  // mova { z0.d-z3.d }, za.d[x9, #0]\n"
      ".inst 0xa060c720  // st1w { z0.s-z3.s }, pn9.b, [x25]\n"
      ".inst 0xc0062c20  // mova { z0.d-z3.d }, za.d[x9, #1]\n"
      ".inst 0xa061c320  // st1w { z0.s-z3.s }, p8, [x25, #0x4, MUL VL]\n"
      "addvl x25, x25, #8\n"
      "19:"  // Width 2: Output done
      "b 36f\n"
      "20:"  // Width 3
      "mov x20, #0x2\n"
      "mov x23, %x[A_ptr]\n"
      "lsl x21, %x[K], #0x2\n"
      "msub x20, x10, x20, %x[N]\n"
      "mov x22, %x[K]\n"
      ".inst 0xf8b54af8  // rprfm pldmany, x21, [x23]\n"
      ".inst 0x25b467f0  // whilelt p8.s, XZR, x20, VLx4\n"
      "cbz x24, 21f\n"
      ".inst 0xa040c71c  // ld1w { z28.s-z31.s }, pn9.b/Z, [x24]\n"
      ".inst 0xc0042f80  // mova za.d[x9, #0], { z28.d-z31.d }\n"
      ".inst 0xa041c704  // ld1w { z4.s-z7.s }, pn9.b/Z, [x24, #0x4, MUL VL]\n"
      ".inst 0xc0042c81  // mova za.d[x9, #1], { z4.d-z7.d }\n"
      ".inst 0xa042c704  // ld1w { z4.s-z7.s }, pn9.b/Z, [x24, #0x8, MUL VL]\n"
      ".inst 0xc0042c82  // mova za.d[x9, #2], { z4.d-z7.d }\n"
      "b 22f\n"
      "21:"  // Width 3: no bias
      ".inst 0xc00800ff  // zero { zad0, zad1, zad2, zad3, zad4, zad5, zad6, zad7 }\n"
      "22:"  // Width 3: setup done
      "cmp x22, #0x8\n"
      "ble 24f\n"
      "23:"  // Width 3: Multiply loop: Main loop head
      "whilelt p1.s, XZR, x22\n"
      "whilelt p0.s, x27, x22\n"
      "ld1rqw { z14.s }, p1/Z, [x23]\n"
      ".inst 0x658aa9ce  // bfcvt z14.h, p2/M, z14.s\n"
      "ld1rqw { z16.s }, p0/Z, [x23, #16]\n"
      ".inst 0x658aaa10  // bfcvt z16.h, p2/M, z16.s\n"
      "uzp1 z14.h, z14.h, z14.h\n"
      "sub x22, x22, #0x8\n"
      "uzp1 z16.h, z16.h, z16.h\n"
      "trn1 z14.d, z14.d, z16.d\n"
      ".inst 0xa040a745  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x26]\n"
      "cmp x22, #0x8\n"
      ".inst 0xa041a759  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15eb098  // bfdot za.s[x9, 0], { z4.h-z7.h }, z14.h[0]\n"
      "add x23, x23, #0x20\n"
      ".inst 0xa042a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15eb319  // bfdot za.s[x9, 1], { z24.h-z27.h }, z14.h[0]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15eb01a  // bfdot za.s[x9, 2], { z0.h-z3.h }, z14.h[0]\n"
      ".inst 0xa040a749  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x26]\n"
      ".inst 0xa041a745  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15eb518  // bfdot za.s[x9, 0], { z8.h-z11.h }, z14.h[1]\n"
      ".inst 0xa042a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15eb499  // bfdot za.s[x9, 1], { z4.h-z7.h }, z14.h[1]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15eb61a  // bfdot za.s[x9, 2], { z16.h-z19.h }, z14.h[1]\n"
      ".inst 0xa040a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26]\n"
      ".inst 0xa041a75d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15eb818  // bfdot za.s[x9, 0], { z0.h-z3.h }, z14.h[2]\n"
      ".inst 0xa042a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15ebb99  // bfdot za.s[x9, 1], { z28.h-z31.h }, z14.h[2]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15eb81a  // bfdot za.s[x9, 2], { z0.h-z3.h }, z14.h[2]\n"
      ".inst 0xa040a759  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x26]\n"
      ".inst 0xa041a75d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15ebf18  // bfdot za.s[x9, 0], { z24.h-z27.h }, z14.h[3]\n"
      ".inst 0xa042a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15ebf99  // bfdot za.s[x9, 1], { z28.h-z31.h }, z14.h[3]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15ebe1a  // bfdot za.s[x9, 2], { z16.h-z19.h }, z14.h[3]\n"
      "bgt 23b\n"
      "24:"  // Width 3: Multiply loop: Single iteration only
      "whilelt p1.s, XZR, x22\n"
      "whilelt p0.s, x27, x22\n"
      "ld1rqw { z15.s }, p1/Z, [x23]\n"
      ".inst 0x658aa9ef  // bfcvt z15.h, p2/M, z15.s\n"
      "ld1rqw { z31.s }, p0/Z, [x23, #16]\n"
      ".inst 0x658aabff  // bfcvt z31.h, p2/M, z31.s\n"
      "uzp1 z15.h, z15.h, z15.h\n"
      "subs x22, x22, #0x2\n"
      "uzp1 z31.h, z31.h, z31.h\n"
      "trn1 z15.d, z15.d, z31.d\n"
      ".inst 0xa040a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26]\n"
      "add x23, x23, #0x20\n"
      ".inst 0xa041a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fb218  // bfdot za.s[x9, 0], { z16.h-z19.h }, z15.h[0]\n"
      ".inst 0xa042a745  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15fb019  // bfdot za.s[x9, 1], { z0.h-z3.h }, z15.h[0]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15fb09a  // bfdot za.s[x9, 2], { z4.h-z7.h }, z15.h[0]\n"
      "ble 25f\n"
      ".inst 0xa040a755  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x26]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xc15fb698  // bfdot za.s[x9, 0], { z20.h-z23.h }, z15.h[1]\n"
      ".inst 0xa041a755  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fb699  // bfdot za.s[x9, 1], { z20.h-z23.h }, z15.h[1]\n"
      ".inst 0xa042a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15fb61a  // bfdot za.s[x9, 2], { z16.h-z19.h }, z15.h[1]\n"
      "addvl x26, x26, #16\n"
      "ble 25f\n"
      ".inst 0xa040a745  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x26]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xc15fb898  // bfdot za.s[x9, 0], { z4.h-z7.h }, z15.h[2]\n"
      ".inst 0xa041a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fb819  // bfdot za.s[x9, 1], { z0.h-z3.h }, z15.h[2]\n"
      ".inst 0xa042a759  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15fbb1a  // bfdot za.s[x9, 2], { z24.h-z27.h }, z15.h[2]\n"
      "addvl x26, x26, #16\n"
      "ble 25f\n"
      ".inst 0xa040a75d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x26]\n"
      ".inst 0xc15fbf98  // bfdot za.s[x9, 0], { z28.h-z31.h }, z15.h[3]\n"
      ".inst 0xa041a749  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fbd19  // bfdot za.s[x9, 1], { z8.h-z11.h }, z15.h[3]\n"
      ".inst 0xa042a745  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15fbc9a  // bfdot za.s[x9, 2], { z4.h-z7.h }, z15.h[3]\n"
      "addvl x26, x26, #16\n"
      "25:"  // Width 3: Multiply loop: multiply skip
      "tbz %x[flags], #1, 26f\n"
      "add x21, %x[args_ptr], %[offset_min]\n"
      "add x20, %x[args_ptr], %[offset_max]\n"
      ".inst 0xc0062c1c  // mova { z28.d-z31.d }, za.d[x9, #0]\n"
      "ld1rw { z17.s }, p2/Z, [x21]\n"
      ".inst 0xc0062c24  // mova { z4.d-z7.d }, za.d[x9, #1]\n"
      "ld1rw { z16.s }, p2/Z, [x20]\n"
      ".inst 0xc1b0ca3c  // fclamp { z28.s-z31.s }, z17.s, z16.s\n"
      ".inst 0xc0062c4c  // mova { z12.d-z15.d }, za.d[x9, #2]\n"
      ".inst 0xa060c73c  // st1w { z28.s-z31.s }, pn9.b, [x25]\n"
      ".inst 0xc1b0ca24  // fclamp { z4.s-z7.s }, z17.s, z16.s\n"
      ".inst 0xa061c724  // st1w { z4.s-z7.s }, pn9.b, [x25, #0x4, MUL VL]\n"
      ".inst 0xc1b0ca2c  // fclamp { z12.s-z15.s }, z17.s, z16.s\n"
      ".inst 0xa062c32c  // st1w { z12.s-z15.s }, p8, [x25, #0x8, MUL VL]\n"
      "addvl x25, x25, #12\n"
      "b 27f\n"
      "26:"  // Width 3: No activation
      ".inst 0xc0062c00  // mova { z0.d-z3.d }, za.d[x9, #0]\n"
      ".inst 0xa060c720  // st1w { z0.s-z3.s }, pn9.b, [x25]\n"
      ".inst 0xc0062c30  // mova { z16.d-z19.d }, za.d[x9, #1]\n"
      ".inst 0xa061c730  // st1w { z16.s-z19.s }, pn9.b, [x25, #0x4, MUL VL]\n"
      ".inst 0xc0062c50  // mova { z16.d-z19.d }, za.d[x9, #2]\n"
      ".inst 0xa062c330  // st1w { z16.s-z19.s }, p8, [x25, #0x8, MUL VL]\n"
      "addvl x25, x25, #12\n"
      "27:"  // Width 3: Output done
      "b 36f\n"
      "28:"  // Width 4
      "mov x20, #0x3\n"
      "mov x23, %x[A_ptr]\n"
      "lsl x21, %x[K], #0x2\n"
      "msub x20, x10, x20, %x[N]\n"
      "mov x22, %x[K]\n"
      ".inst 0xf8b54af8  // rprfm pldmany, x21, [x23]\n"
      ".inst 0x25b467f0  // whilelt p8.s, XZR, x20, VLx4\n"
      "cbz x24, 29f\n"
      ".inst 0xa040c70c  // ld1w { z12.s-z15.s }, pn9.b/Z, [x24]\n"
      ".inst 0xc0042d80  // mova za.d[x9, #0], { z12.d-z15.d }\n"
      ".inst 0xa041c70c  // ld1w { z12.s-z15.s }, pn9.b/Z, [x24, #0x4, MUL VL]\n"
      ".inst 0xc0042d81  // mova za.d[x9, #1], { z12.d-z15.d }\n"
      ".inst 0xa042c710  // ld1w { z16.s-z19.s }, pn9.b/Z, [x24, #0x8, MUL VL]\n"
      ".inst 0xc0042e02  // mova za.d[x9, #2], { z16.d-z19.d }\n"
      ".inst 0xa043c714  // ld1w { z20.s-z23.s }, pn9.b/Z, [x24, #0xc, MUL VL]\n"
      ".inst 0xc0042e83  // mova za.d[x9, #3], { z20.d-z23.d }\n"
      "addvl x24, x24, #16\n"
      "b 30f\n"
      "29:"  // Width 4: no bias
      ".inst 0xc00800ff  // zero { zad0, zad1, zad2, zad3, zad4, zad5, zad6, zad7 }\n"
      "30:"  // Width 4: setup done
      "cmp x22, #0x8\n"
      "ble 32f\n"
      "31:"  // Width 4: Multiply loop: Main loop head
      "whilelt p1.s, XZR, x22\n"
      "whilelt p0.s, x27, x22\n"
      "ld1rqw { z6.s }, p1/Z, [x23]\n"
      ".inst 0x658aa8c6  // bfcvt z6.h, p2/M, z6.s\n"
      "ld1rqw { z16.s }, p0/Z, [x23, #16]\n"
      ".inst 0x658aaa10  // bfcvt z16.h, p2/M, z16.s\n"
      "uzp1 z6.h, z6.h, z6.h\n"
      "sub x22, x22, #0x8\n"
      "uzp1 z16.h, z16.h, z16.h\n"
      "trn1 z6.d, z6.d, z16.d\n"
      ".inst 0xa040a74d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x26]\n"
      "cmp x22, #0x8\n"
      ".inst 0xa041a749  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc156b198  // bfdot za.s[x9, 0], { z12.h-z15.h }, z6.h[0]\n"
      "add x23, x23, #0x20\n"
      ".inst 0xa042a74d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc156b119  // bfdot za.s[x9, 1], { z8.h-z11.h }, z6.h[0]\n"
      ".inst 0xa043a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0xc, MUL VL]\n"
      ".inst 0xc156b19a  // bfdot za.s[x9, 2], { z12.h-z15.h }, z6.h[0]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc156b21b  // bfdot za.s[x9, 3], { z16.h-z19.h }, z6.h[0]\n"
      ".inst 0xa040a749  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x26]\n"
      ".inst 0xa041a74d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc156b518  // bfdot za.s[x9, 0], { z8.h-z11.h }, z6.h[1]\n"
      ".inst 0xa042a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc156b599  // bfdot za.s[x9, 1], { z12.h-z15.h }, z6.h[1]\n"
      ".inst 0xa043a755  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x26, #0xc, MUL VL]\n"
      ".inst 0xc156b41a  // bfdot za.s[x9, 2], { z0.h-z3.h }, z6.h[1]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc156b69b  // bfdot za.s[x9, 3], { z20.h-z23.h }, z6.h[1]\n"
      ".inst 0xa040a749  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x26]\n"
      ".inst 0xa041a74d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc156b918  // bfdot za.s[x9, 0], { z8.h-z11.h }, z6.h[2]\n"
      ".inst 0xa042a749  // ldnt1h { z8.h-z11.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc156b999  // bfdot za.s[x9, 1], { z12.h-z15.h }, z6.h[2]\n"
      ".inst 0xa043a755  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x26, #0xc, MUL VL]\n"
      ".inst 0xc156b91a  // bfdot za.s[x9, 2], { z8.h-z11.h }, z6.h[2]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc156ba9b  // bfdot za.s[x9, 3], { z20.h-z23.h }, z6.h[2]\n"
      ".inst 0xa040a75d  // ldnt1h { z28.h-z31.h }, pn9.b/Z, [x26]\n"
      ".inst 0xa041a74d  // ldnt1h { z12.h-z15.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc156bf98  // bfdot za.s[x9, 0], { z28.h-z31.h }, z6.h[3]\n"
      ".inst 0xa042a759  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc156bd99  // bfdot za.s[x9, 1], { z12.h-z15.h }, z6.h[3]\n"
      ".inst 0xa043a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0xc, MUL VL]\n"
      ".inst 0xc156bf1a  // bfdot za.s[x9, 2], { z24.h-z27.h }, z6.h[3]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc156be1b  // bfdot za.s[x9, 3], { z16.h-z19.h }, z6.h[3]\n"
      "bgt 31b\n"
      "32:"  // Width 4: Multiply loop: Single iteration only
      "whilelt p1.s, XZR, x22\n"
      "whilelt p0.s, x27, x22\n"
      "ld1rqw { z15.s }, p1/Z, [x23]\n"
      ".inst 0x658aa9ef  // bfcvt z15.h, p2/M, z15.s\n"
      "ld1rqw { z16.s }, p0/Z, [x23, #16]\n"
      ".inst 0x658aaa10  // bfcvt z16.h, p2/M, z16.s\n"
      "uzp1 z15.h, z15.h, z15.h\n"
      "subs x22, x22, #0x2\n"
      "uzp1 z16.h, z16.h, z16.h\n"
      "trn1 z15.d, z15.d, z16.d\n"
      ".inst 0xa040a759  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x26]\n"
      "add x23, x23, #0x20\n"
      ".inst 0xa041a745  // ldnt1h { z4.h-z7.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fb318  // bfdot za.s[x9, 0], { z24.h-z27.h }, z15.h[0]\n"
      ".inst 0xa042a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15fb099  // bfdot za.s[x9, 1], { z4.h-z7.h }, z15.h[0]\n"
      ".inst 0xa043a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0xc, MUL VL]\n"
      ".inst 0xc15fb01a  // bfdot za.s[x9, 2], { z0.h-z3.h }, z15.h[0]\n"
      "addvl x26, x26, #16\n"
      ".inst 0xc15fb21b  // bfdot za.s[x9, 3], { z16.h-z19.h }, z15.h[0]\n"
      "ble 33f\n"
      ".inst 0xa040a759  // ldnt1h { z24.h-z27.h }, pn9.b/Z, [x26]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xc15fb718  // bfdot za.s[x9, 0], { z24.h-z27.h }, z15.h[1]\n"
      ".inst 0xa041a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fb619  // bfdot za.s[x9, 1], { z16.h-z19.h }, z15.h[1]\n"
      ".inst 0xa042a755  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15fb69a  // bfdot za.s[x9, 2], { z20.h-z23.h }, z15.h[1]\n"
      ".inst 0xa043a741  // ldnt1h { z0.h-z3.h }, pn9.b/Z, [x26, #0xc, MUL VL]\n"
      ".inst 0xc15fb41b  // bfdot za.s[x9, 3], { z0.h-z3.h }, z15.h[1]\n"
      "addvl x26, x26, #16\n"
      "ble 33f\n"
      ".inst 0xa040a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26]\n"
      "subs x22, x22, #0x2\n"
      ".inst 0xc15fba18  // bfdot za.s[x9, 0], { z16.h-z19.h }, z15.h[2]\n"
      ".inst 0xa041a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fba19  // bfdot za.s[x9, 1], { z16.h-z19.h }, z15.h[2]\n"
      ".inst 0xa042a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15fba1a  // bfdot za.s[x9, 2], { z16.h-z19.h }, z15.h[2]\n"
      ".inst 0xa043a755  // ldnt1h { z20.h-z23.h }, pn9.b/Z, [x26, #0xc, MUL VL]\n"
      ".inst 0xc15fba9b  // bfdot za.s[x9, 3], { z20.h-z23.h }, z15.h[2]\n"
      "addvl x26, x26, #16\n"
      "ble 33f\n"
      ".inst 0xa040a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26]\n"
      ".inst 0xc15fbe18  // bfdot za.s[x9, 0], { z16.h-z19.h }, z15.h[3]\n"
      ".inst 0xa041a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0x4, MUL VL]\n"
      ".inst 0xc15fbe19  // bfdot za.s[x9, 1], { z16.h-z19.h }, z15.h[3]\n"
      ".inst 0xa042a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0x8, MUL VL]\n"
      ".inst 0xc15fbe1a  // bfdot za.s[x9, 2], { z16.h-z19.h }, z15.h[3]\n"
      ".inst 0xa043a751  // ldnt1h { z16.h-z19.h }, pn9.b/Z, [x26, #0xc, MUL VL]\n"
      ".inst 0xc15fbe1b  // bfdot za.s[x9, 3], { z16.h-z19.h }, z15.h[3]\n"
      "addvl x26, x26, #16\n"
      "33:"  // Width 4: Multiply loop: multiply skip
      "tbz %x[flags], #1, 34f\n"
      "add x21, %x[args_ptr], %[offset_min]\n"
      "add x20, %x[args_ptr], %[offset_max]\n"
      ".inst 0xc0062c0c  // mova { z12.d-z15.d }, za.d[x9, #0]\n"
      "ld1rw { z21.s }, p2/Z, [x21]\n"
      ".inst 0xc0062c38  // mova { z24.d-z27.d }, za.d[x9, #1]\n"
      "ld1rw { z20.s }, p2/Z, [x20]\n"
      ".inst 0xc1b4caac  // fclamp { z12.s-z15.s }, z21.s, z20.s\n"
      ".inst 0xc0062c40  // mova { z0.d-z3.d }, za.d[x9, #2]\n"
      ".inst 0xa060c72c  // st1w { z12.s-z15.s }, pn9.b, [x25]\n"
      ".inst 0xc1b4cab8  // fclamp { z24.s-z27.s }, z21.s, z20.s\n"
      ".inst 0xc0062c70  // mova { z16.d-z19.d }, za.d[x9, #3]\n"
      ".inst 0xa061c738  // st1w { z24.s-z27.s }, pn9.b, [x25, #0x4, MUL VL]\n"
      ".inst 0xc1b4caa0  // fclamp { z0.s-z3.s }, z21.s, z20.s\n"
      ".inst 0xa062c720  // st1w { z0.s-z3.s }, pn9.b, [x25, #0x8, MUL VL]\n"
      ".inst 0xc1b4cab0  // fclamp { z16.s-z19.s }, z21.s, z20.s\n"
      ".inst 0xa063c330  // st1w { z16.s-z19.s }, p8, [x25, #0xc, MUL VL]\n"
      "addvl x25, x25, #16\n"
      "b 35f\n"
      "34:"  // Width 4: No activation
      ".inst 0xc0062c10  // mova { z16.d-z19.d }, za.d[x9, #0]\n"
      ".inst 0xa060c730  // st1w { z16.s-z19.s }, pn9.b, [x25]\n"
      ".inst 0xc0062c30  // mova { z16.d-z19.d }, za.d[x9, #1]\n"
      ".inst 0xa061c730  // st1w { z16.s-z19.s }, pn9.b, [x25, #0x4, MUL VL]\n"
      ".inst 0xc0062c54  // mova { z20.d-z23.d }, za.d[x9, #2]\n"
      ".inst 0xa062c734  // st1w { z20.s-z23.s }, pn9.b, [x25, #0x8, MUL VL]\n"
      ".inst 0xc0062c78  // mova { z24.d-z27.d }, za.d[x9, #3]\n"
      ".inst 0xa063c338  // st1w { z24.s-z27.s }, p8, [x25, #0xc, MUL VL]\n"
      "addvl x25, x25, #16\n"
      "35:"  // Width 4: Output done
      "subs x28, x28, #0x4\n"
      "sub %x[N], %x[N], x10, LSL #2\n"
      "bgt 4b\n"
      "36:"  // Exit
      ".inst 0xd503467f  // SMSTOP\n"
      "ptrue p8.b\n"
      : [N] "+&r" (N)
      : [A_ptr] "r" (A_ptr), [B_ptr] "r" (B_ptr), [K] "r" (K), [args_ptr] "r" (&ka), [bias] "r" (bias), [flags] "r" (flags), [offset_max] "I" (offsetof(KernelArgs, maxval)), [offset_min] "I" (offsetof(KernelArgs, minval)), [output_ptr] "r" (output_ptr)
      : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "x9", "x10", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
    );
}

} // namespace arm_gemm

#endif  // defined(ARM_COMPUTE_ENABLE_SME2)
