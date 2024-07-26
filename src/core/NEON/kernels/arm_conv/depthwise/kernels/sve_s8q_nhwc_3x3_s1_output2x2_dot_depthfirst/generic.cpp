/*
 * Copyright (c) 2021-2024 Arm Limited.
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

#if defined(ARM_COMPUTE_ENABLE_SVE)

#include "arm_gemm.hpp"
#include <cstdint>

namespace arm_conv {
namespace depthwise {

void sve_s8q_nhwc_3x3_s1_output2x2_dot_depthfirst_impl(const unsigned int n_channels, const int8_t *const *const inptrs, const int8_t *params, const int32_t *, const arm_gemm::Requantize32& qp, const int32_t *, const int32_t *, int8_t *const *const outptrs)
{
  __asm__ __volatile__(
    "mov x14, #0x0\n"
    "whilelt p0.b, x14, %x[n_channels]\n"
    "ldp x27, x26, [%x[inptrs], #0x0]\n"
    "ldp x25, x24, [%x[inptrs], #0x10]\n"
    "ldp x23, x22, [%x[inptrs], #0x20]\n"
    "ldp x13, x21, [%x[inptrs], #0x30]\n"
    "mov x20, #0x1\n"
    "ptrue p2.b\n"
    "ldp x12, x11, [%x[outptrs], #0x0]\n"
    "ldp x10, x9, [%x[outptrs], #0x10]\n"
    "orr x20, x20, #0x100\n"
    "orr x20, x20, #0x10000\n"
    "ld1b { z15.b }, p0/Z, [x27, x14]\n"
    "ld1b { z21.b }, p0/Z, [x26, x14]\n"
    "dup z25.s, w20\n"
    "mov x28, #0x0\n"
    "ldp x27, x26, [%x[inptrs], #0x40]\n"
    "ld1b { z31.b }, p0/Z, [x25, x14]\n"
    "zip2 z16.b, z15.b, z31.b\n"
    "zip1 z15.b, z15.b, z31.b\n"
    "ld1b { z29.b }, p0/Z, [x24, x14]\n"
    "ldp x25, x24, [%x[inptrs], #0x50]\n"
    "zip1 z30.b, z21.b, z29.b\n"
    "zip2 z29.b, z21.b, z29.b\n"
    "ld1b { z9.b }, p0/Z, [x23, x14]\n"
    "ld1b { z20.b }, p0/Z, [x22, x14]\n"
    "zip2 z13.b, z15.b, z30.b\n"
    "zip1 z15.b, z15.b, z30.b\n"
    "ldp x23, x22, [%x[inptrs], #0x60]\n"
    "ld1b { z5.b }, p0/Z, [x13, x14]\n"
    "zip1 z14.b, z16.b, z29.b\n"
    "zip2 z29.b, z16.b, z29.b\n"
    "ld1b { z17.b }, p0/Z, [x21, x14]\n"
    "ldp x21, x20, [%x[inptrs], #0x70]\n"
    "zip2 z31.b, z9.b, z5.b\n"
    "zip1 z9.b, z9.b, z5.b\n"
    "ld1b { z18.b }, p0/Z, [x27, x14]\n"
    "ld1b { z28.b }, p0/Z, [x26, x14]\n"
    "zip1 z21.b, z20.b, z17.b\n"
    "zip2 z17.b, z20.b, z17.b\n"
    "ld1b { z6.b }, p0/Z, [x25, x14]\n"
    "ld1b { z4.b }, p0/Z, [x24, x14]\n"
    "zip2 z23.b, z18.b, z6.b\n"
    "zip1 z18.b, z18.b, z6.b\n"
    "ld1b { z2.b }, p0/Z, [x23, x14]\n"
    "ld1b { z19.b }, p0/Z, [x22, x14]\n"
    "zip1 z24.b, z28.b, z4.b\n"
    "zip2 z4.b, z28.b, z4.b\n"
    "ld1b { z16.b }, p0/Z, [x21, x14]\n"
    "ld1b { z5.b }, p0/Z, [x20, x14]\n"
    "zip2 z22.b, z2.b, z16.b\n"
    "zip1 z2.b, z2.b, z16.b\n"
    "zip1 z0.b, z19.b, z5.b\n"
    "zip2 z5.b, z19.b, z5.b\n"
    "ld1w { z10.s }, p2/Z, [%x[params]]\n"
    "ld1rw { z7.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_minval]]\n"
    "ld1rw { z6.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_maxval]]\n"
    "ld1rw { z8.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_b_offset]]\n"
    "zip2 z19.b, z9.b, z21.b\n"
    "zip1 z9.b, z9.b, z21.b\n"
    "ld1rw { z16.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_c_offset]]\n"
    "ldp x27, x26, [%x[inptrs], #0x0]\n"
    "zip1 z11.b, z31.b, z17.b\n"
    "zip2 z17.b, z31.b, z17.b\n"
    "ldp x25, x23, [%x[inptrs], #0x10]\n"
    "ldp x24, x22, [%x[inptrs], #0x20]\n"
    "zip2 z12.b, z18.b, z24.b\n"
    "zip1 z18.b, z18.b, z24.b\n"
    "ldp x21, x20, [%x[inptrs], #0x30]\n"
    "zip1 z20.b, z23.b, z4.b\n"
    "zip2 z4.b, z23.b, z4.b\n"
    "ld1b { z26.b }, p2/Z, [%x[params], #1, MUL VL]\n"
    "zip2 z24.b, z2.b, z0.b\n"
    "zip1 z2.b, z2.b, z0.b\n"
    "ld1b { z3.b }, p2/Z, [%x[params], #2, MUL VL]\n"
    "ld1b { z1.b }, p2/Z, [%x[params], #3, MUL VL]\n"
    "zip1 z0.b, z22.b, z5.b\n"
    "zip2 z5.b, z22.b, z5.b\n"
    "addvl %x[params], %x[params], #4\n"
    "mov z22.d, z10.d\n"
    "mov z31.d, z10.d\n"
    "mov z21.d, z10.d\n"
    "1:"  // Loop
    "mov z30.s, #0x0\n"
    "sdot z30.s, z25.b, z9.b\n"
    "sdot z10.s, z26.b, z15.b\n"
    "whilelt p0.s, x28, %x[n_channels]\n"
    "sdot z30.s, z25.b, z18.b\n"
    "sdot z31.s, z26.b, z9.b\n"
    "mov z27.s, #0x0\n"
    "incw x14, ALL, MUL #4\n"
    "sdot z10.s, z3.b, z9.b\n"
    "ext z9.b, z9.b, z9.b, #0x1\n"
    "movprfx z28, z30\n sdot z28.s, z25.b, z2.b\n"
    "sdot z30.s, z25.b, z15.b\n"
    "ext z15.b, z15.b, z15.b, #0x1\n"
    "sdot z27.s, z25.b, z9.b\n"
    "sdot z31.s, z3.b, z18.b\n"
    "sdot z10.s, z1.b, z18.b\n"
    "ext z18.b, z18.b, z18.b, #0x1\n"
    "sdot z22.s, z26.b, z15.b\n"
    "sdot z21.s, z26.b, z9.b\n"
    "sdot z27.s, z25.b, z18.b\n"
    "sdot z31.s, z1.b, z2.b\n"
    "ext z2.b, z2.b, z2.b, #0x1\n"
    "sdot z22.s, z3.b, z9.b\n"
    "sdot z21.s, z3.b, z18.b\n"
    "ld1w { z3.s }, p2/Z, [%x[params], #1, MUL VL]\n"
    "mls z10.s, p2/M, z30.s, z8.s\n"
    "movprfx z26, z27\n sdot z26.s, z25.b, z2.b\n"
    "mov z9.s, #0x0\n"
    "sdot z27.s, z25.b, z15.b\n"
    "ld1w { z23.s }, p2/Z, [%x[params]]\n"
    "sdot z22.s, z1.b, z18.b\n"
    ".inst 0x04b7754a  // sqrdmulh z10.s, z10.s, z23.s\n"
    "sdot z21.s, z1.b, z2.b\n"
    "mls z22.s, p2/M, z27.s, z8.s\n"
    "and z18.d, z10.d, z3.d\n"
    "mls z31.s, p2/M, z28.s, z8.s\n"
    "mls z21.s, p2/M, z26.s, z8.s\n"
    "asr z18.s, z18.s, #0x1f\n"
    ".inst 0x04b776d6  // sqrdmulh z22.s, z22.s, z23.s\n"
    ".inst 0x04b777ff  // sqrdmulh z31.s, z31.s, z23.s\n"
    "sdot z9.s, z25.b, z19.b\n"
    ".inst 0x04b776b5  // sqrdmulh z21.s, z21.s, z23.s\n"
    "sqadd z10.s, z10.s, z18.s\n"
    ".inst 0x4482886a  // srshl z10.s, p2/M, z10.s, z3.s\n"
    "sdot z9.s, z25.b, z12.b\n"
    "and z28.d, z22.d, z3.d\n"
    "and z23.d, z31.d, z3.d\n"
    "movprfx z27, z9\n sdot z27.s, z25.b, z24.b\n"
    "ld1w { z30.s }, p2/Z, [%x[params], #6, MUL VL]\n"
    "and z18.d, z21.d, z3.d\n"
    "asr z28.s, z28.s, #0x1f\n"
    "sdot z9.s, z25.b, z13.b\n"
    "asr z23.s, z23.s, #0x1f\n"
    "asr z18.s, z18.s, #0x1f\n"
    "sqadd z22.s, z22.s, z28.s\n"
    "sqadd z31.s, z31.s, z23.s\n"
    ".inst 0x44828876  // srshl z22.s, p2/M, z22.s, z3.s\n"
    ".inst 0x4482887f  // srshl z31.s, p2/M, z31.s, z3.s\n"
    "sqadd z21.s, z21.s, z18.s\n"
    "add z10.s, z10.s, z16.s\n"
    ".inst 0x44828875  // srshl z21.s, p2/M, z21.s, z3.s\n"
    "smax z10.s, p2/M, z10.s, z7.s\n"
    "add z22.s, z22.s, z16.s\n"
    "add z31.s, z31.s, z16.s\n"
    "smin z10.s, p2/M, z10.s, z6.s\n"
    "smax z22.s, p2/M, z22.s, z7.s\n"
    "add z21.s, z21.s, z16.s\n"
    "smax z31.s, p2/M, z31.s, z7.s\n"
    "smax z21.s, p2/M, z21.s, z7.s\n"
    "st1b { z10.s }, p0, [x12, x28]\n"
    "ld1w { z28.s }, p2/Z, [%x[params], #2, MUL VL]\n"
    "ld1b { z1.b }, p2/Z, [%x[params], #3, MUL VL]\n"
    "smin z22.s, p2/M, z22.s, z6.s\n"
    "smin z31.s, p2/M, z31.s, z6.s\n"
    "smin z21.s, p2/M, z21.s, z6.s\n"
    "st1b { z22.s }, p0, [x11, x28]\n"
    "mov z26.d, z28.d\n"
    "ld1b { z15.b }, p2/Z, [%x[params], #4, MUL VL]\n"
    "st1b { z31.s }, p0, [x10, x28]\n"
    "mov z31.d, z28.d\n"
    "sdot z31.s, z1.b, z19.b\n"
    "ld1b { z23.b }, p2/Z, [%x[params], #5, MUL VL]\n"
    "st1b { z21.s }, p0, [x9, x28]\n"
    "mov z22.d, z28.d\n"
    "sdot z28.s, z1.b, z13.b\n"
    "sdot z28.s, z15.b, z19.b\n"
    "ext z13.b, z13.b, z13.b, #0x1\n"
    "ext z19.b, z19.b, z19.b, #0x1\n"
    "sdot z26.s, z1.b, z13.b\n"
    "ld1w { z21.s }, p2/Z, [%x[params], #7, MUL VL]\n"
    "mov z18.s, #0x0\n"
    "sdot z22.s, z1.b, z19.b\n"
    "sdot z18.s, z25.b, z19.b\n"
    "incw x28\n"
    "sdot z31.s, z15.b, z12.b\n"
    "sdot z28.s, z23.b, z12.b\n"
    "ext z12.b, z12.b, z12.b, #0x1\n"
    "whilelt p0.s, x28, %x[n_channels]\n"
    "sdot z26.s, z15.b, z19.b\n"
    "sdot z22.s, z15.b, z12.b\n"
    "addvl %x[params], %x[params], #16\n"
    "sdot z18.s, z25.b, z12.b\n"
    "sdot z31.s, z23.b, z24.b\n"
    "ext z24.b, z24.b, z24.b, #0x1\n"
    "mls z28.s, p2/M, z9.s, z8.s\n"
    "sdot z26.s, z23.b, z12.b\n"
    ".inst 0x04be779c  // sqrdmulh z28.s, z28.s, z30.s\n"
    "sdot z22.s, z23.b, z24.b\n"
    "movprfx z12, z18\n sdot z12.s, z25.b, z24.b\n"
    "and z2.d, z28.d, z21.d\n"
    "sdot z18.s, z25.b, z13.b\n"
    "mls z26.s, p2/M, z18.s, z8.s\n"
    "asr z2.s, z2.s, #0x1f\n"
    "mls z31.s, p2/M, z27.s, z8.s\n"
    "mls z22.s, p2/M, z12.s, z8.s\n"
    ".inst 0x04be775a  // sqrdmulh z26.s, z26.s, z30.s\n"
    ".inst 0x04be77ff  // sqrdmulh z31.s, z31.s, z30.s\n"
    ".inst 0x04be76d6  // sqrdmulh z22.s, z22.s, z30.s\n"
    "ld1w { z1.s }, p2/Z, [%x[params], #-4, MUL VL]\n"
    "sqadd z28.s, z28.s, z2.s\n"
    "and z24.d, z26.d, z21.d\n"
    ".inst 0x44828abc  // srshl z28.s, p2/M, z28.s, z21.s\n"
    "and z23.d, z31.d, z21.d\n"
    "and z18.d, z22.d, z21.d\n"
    "asr z24.s, z24.s, #0x1f\n"
    "asr z23.s, z23.s, #0x1f\n"
    "asr z18.s, z18.s, #0x1f\n"
    "sqadd z26.s, z26.s, z24.s\n"
    ".inst 0x44828aba  // srshl z26.s, p2/M, z26.s, z21.s\n"
    "ld1b { z30.b }, p2/Z, [%x[params], #-6, MUL VL]\n"
    "sqadd z31.s, z31.s, z23.s\n"
    "sqadd z22.s, z22.s, z18.s\n"
    ".inst 0x44828abf  // srshl z31.s, p2/M, z31.s, z21.s\n"
    ".inst 0x44828ab6  // srshl z22.s, p2/M, z22.s, z21.s\n"
    "add z28.s, z28.s, z16.s\n"
    "smax z28.s, p2/M, z28.s, z7.s\n"
    "add z26.s, z26.s, z16.s\n"
    "smin z28.s, p2/M, z28.s, z6.s\n"
    "add z31.s, z31.s, z16.s\n"
    "add z22.s, z22.s, z16.s\n"
    "smax z26.s, p2/M, z26.s, z7.s\n"
    "smax z31.s, p2/M, z31.s, z7.s\n"
    "mov z24.s, #0x0\n"
    "sdot z24.s, z25.b, z11.b\n"
    "smax z22.s, p2/M, z22.s, z7.s\n"
    "st1b { z28.s }, p0, [x12, x28]\n"
    "ld1w { z23.s }, p2/Z, [%x[params], #-8, MUL VL]\n"
    "ld1b { z19.b }, p2/Z, [%x[params], #-7, MUL VL]\n"
    "smin z26.s, p2/M, z26.s, z6.s\n"
    "smin z31.s, p2/M, z31.s, z6.s\n"
    "smin z22.s, p2/M, z22.s, z6.s\n"
    "st1b { z26.s }, p0, [x11, x28]\n"
    "mov z28.d, z23.d\n"
    "sdot z24.s, z25.b, z20.b\n"
    "st1b { z31.s }, p0, [x10, x28]\n"
    "mov z27.d, z23.d\n"
    "sdot z27.s, z19.b, z11.b\n"
    "movprfx z13, z24\n sdot z13.s, z25.b, z0.b\n"
    "st1b { z22.s }, p0, [x9, x28]\n"
    "mov z26.d, z23.d\n"
    "sdot z23.s, z19.b, z14.b\n"
    "sdot z23.s, z30.b, z11.b\n"
    "sdot z24.s, z25.b, z14.b\n"
    "ext z14.b, z14.b, z14.b, #0x1\n"
    "ld1b { z21.b }, p2/Z, [%x[params], #-5, MUL VL]\n"
    "sdot z28.s, z19.b, z14.b\n"
    "ext z11.b, z11.b, z11.b, #0x1\n"
    "mov z12.s, #0x0\n"
    "sdot z26.s, z19.b, z11.b\n"
    "ld1w { z22.s }, p2/Z, [%x[params], #-3, MUL VL]\n"
    "sdot z12.s, z25.b, z11.b\n"
    "sdot z27.s, z30.b, z20.b\n"
    "incw x28\n"
    "whilelt p0.s, x28, %x[n_channels]\n"
    "sdot z23.s, z21.b, z20.b\n"
    "ext z20.b, z20.b, z20.b, #0x1\n"
    "sdot z28.s, z30.b, z11.b\n"
    "sdot z26.s, z30.b, z20.b\n"
    "sdot z12.s, z25.b, z20.b\n"
    "sdot z27.s, z21.b, z0.b\n"
    "ext z0.b, z0.b, z0.b, #0x1\n"
    "mls z23.s, p2/M, z24.s, z8.s\n"
    "sdot z28.s, z21.b, z20.b\n"
    "sdot z26.s, z21.b, z0.b\n"
    ".inst 0x04a176f7  // sqrdmulh z23.s, z23.s, z1.s\n"
    "movprfx z19, z12\n sdot z19.s, z25.b, z0.b\n"
    "sdot z12.s, z25.b, z14.b\n"
    "and z18.d, z23.d, z22.d\n"
    "mls z28.s, p2/M, z12.s, z8.s\n"
    "mls z27.s, p2/M, z13.s, z8.s\n"
    "asr z18.s, z18.s, #0x1f\n"
    "mls z26.s, p2/M, z19.s, z8.s\n"
    ".inst 0x04a1779c  // sqrdmulh z28.s, z28.s, z1.s\n"
    ".inst 0x04a1777b  // sqrdmulh z27.s, z27.s, z1.s\n"
    ".inst 0x04a1775a  // sqrdmulh z26.s, z26.s, z1.s\n"
    "ld1w { z2.s }, p2/Z, [%x[params], #2, MUL VL]\n"
    "sqadd z23.s, z23.s, z18.s\n"
    "and z20.d, z28.d, z22.d\n"
    ".inst 0x44828ad7  // srshl z23.s, p2/M, z23.s, z22.s\n"
    "and z19.d, z27.d, z22.d\n"
    "and z18.d, z26.d, z22.d\n"
    "asr z20.s, z20.s, #0x1f\n"
    "asr z19.s, z19.s, #0x1f\n"
    "asr z18.s, z18.s, #0x1f\n"
    "sqadd z28.s, z28.s, z20.s\n"
    ".inst 0x44828adc  // srshl z28.s, p2/M, z28.s, z22.s\n"
    "ld1b { z13.b }, p2/Z, [%x[params]]\n"
    "sqadd z27.s, z27.s, z19.s\n"
    "sqadd z26.s, z26.s, z18.s\n"
    ".inst 0x44828adb  // srshl z27.s, p2/M, z27.s, z22.s\n"
    ".inst 0x44828ada  // srshl z26.s, p2/M, z26.s, z22.s\n"
    "add z23.s, z23.s, z16.s\n"
    "smax z23.s, p2/M, z23.s, z7.s\n"
    "add z28.s, z28.s, z16.s\n"
    "smin z23.s, p2/M, z23.s, z6.s\n"
    "add z27.s, z27.s, z16.s\n"
    "add z26.s, z26.s, z16.s\n"
    "smax z28.s, p2/M, z28.s, z7.s\n"
    "smax z27.s, p2/M, z27.s, z7.s\n"
    "mov z24.s, #0x0\n"
    "sdot z24.s, z25.b, z17.b\n"
    "smax z26.s, p2/M, z26.s, z7.s\n"
    "st1b { z23.s }, p0, [x12, x28]\n"
    "ld1w { z1.s }, p2/Z, [%x[params], #-2, MUL VL]\n"
    "ld1b { z21.b }, p2/Z, [%x[params], #-1, MUL VL]\n"
    "smin z28.s, p2/M, z28.s, z6.s\n"
    "smin z27.s, p2/M, z27.s, z6.s\n"
    "smin z26.s, p2/M, z26.s, z6.s\n"
    "st1b { z28.s }, p0, [x11, x28]\n"
    "mov z0.d, z1.d\n"
    "sdot z24.s, z25.b, z4.b\n"
    "st1b { z27.s }, p0, [x10, x28]\n"
    "mov z31.d, z1.d\n"
    "sdot z31.s, z21.b, z17.b\n"
    "movprfx z23, z24\n sdot z23.s, z25.b, z5.b\n"
    "st1b { z26.s }, p0, [x9, x28]\n"
    "mov z30.d, z1.d\n"
    "sdot z1.s, z21.b, z29.b\n"
    "sdot z1.s, z13.b, z17.b\n"
    "sdot z24.s, z25.b, z29.b\n"
    "ext z29.b, z29.b, z29.b, #0x1\n"
    "ld1b { z20.b }, p2/Z, [%x[params], #1, MUL VL]\n"
    "sdot z0.s, z21.b, z29.b\n"
    "ext z17.b, z17.b, z17.b, #0x1\n"
    "mov z19.s, #0x0\n"
    "sdot z30.s, z21.b, z17.b\n"
    "ld1w { z22.s }, p2/Z, [%x[params], #3, MUL VL]\n"
    "sdot z19.s, z25.b, z17.b\n"
    "sdot z31.s, z13.b, z4.b\n"
    "incw x28\n"
    "whilelt p1.s, x28, %x[n_channels]\n"
    "sdot z1.s, z20.b, z4.b\n"
    "ext z4.b, z4.b, z4.b, #0x1\n"
    "sdot z0.s, z13.b, z17.b\n"
    "whilelt p0.b, x14, %x[n_channels]\n"
    "sdot z30.s, z13.b, z4.b\n"
    "sdot z19.s, z25.b, z4.b\n"
    "ld1b { z13.b }, p0/Z, [x26, x14]\n"
    "ld1b { z28.b }, p0/Z, [x25, x14]\n"
    "sdot z31.s, z20.b, z5.b\n"
    "ext z5.b, z5.b, z5.b, #0x1\n"
    "mls z1.s, p2/M, z24.s, z8.s\n"
    "ld1b { z27.b }, p0/Z, [x22, x14]\n"
    "sdot z0.s, z20.b, z4.b\n"
    "sdot z30.s, z20.b, z5.b\n"
    ".inst 0x04a27421  // sqrdmulh z1.s, z1.s, z2.s\n"
    "ld1b { z26.b }, p0/Z, [x21, x14]\n"
    "movprfx z18, z19\n sdot z18.s, z25.b, z5.b\n"
    "sdot z19.s, z25.b, z29.b\n"
    "and z11.d, z1.d, z22.d\n"
    "ld1b { z29.b }, p0/Z, [x23, x14]\n"
    "mls z0.s, p2/M, z19.s, z8.s\n"
    "mls z31.s, p2/M, z23.s, z8.s\n"
    "asr z11.s, z11.s, #0x1f\n"
    "ld1b { z17.b }, p0/Z, [x20, x14]\n"
    "mls z30.s, p2/M, z18.s, z8.s\n"
    ".inst 0x04a27400  // sqrdmulh z0.s, z0.s, z2.s\n"
    ".inst 0x04a277ff  // sqrdmulh z31.s, z31.s, z2.s\n"
    ".inst 0x04a277de  // sqrdmulh z30.s, z30.s, z2.s\n"
    "ld1b { z15.b }, p0/Z, [x27, x14]\n"
    "ldp x23, x22, [%x[inptrs], #0x40]\n"
    "sqadd z1.s, z1.s, z11.s\n"
    "and z21.d, z0.d, z22.d\n"
    ".inst 0x44828ac1  // srshl z1.s, p2/M, z1.s, z22.s\n"
    "ldp x21, x20, [%x[inptrs], #0x50]\n"
    "and z20.d, z31.d, z22.d\n"
    "and z19.d, z30.d, z22.d\n"
    "ld1b { z18.b }, p0/Z, [x23, x14]\n"
    "ld1b { z11.b }, p0/Z, [x22, x14]\n"
    "asr z21.s, z21.s, #0x1f\n"
    "asr z20.s, z20.s, #0x1f\n"
    "ld1b { z24.b }, p0/Z, [x21, x14]\n"
    "ld1b { z4.b }, p0/Z, [x20, x14]\n"
    "asr z19.s, z19.s, #0x1f\n"
    "sqadd z0.s, z0.s, z21.s\n"
    ".inst 0x44828ac0  // srshl z0.s, p2/M, z0.s, z22.s\n"
    "ld1b { z3.b }, p2/Z, [%x[params], #6, MUL VL]\n"
    "sqadd z31.s, z31.s, z20.s\n"
    "sqadd z30.s, z30.s, z19.s\n"
    ".inst 0x44828adf  // srshl z31.s, p2/M, z31.s, z22.s\n"
    ".inst 0x44828ade  // srshl z30.s, p2/M, z30.s, z22.s\n"
    "add z1.s, z1.s, z16.s\n"
    "smax z1.s, p2/M, z1.s, z7.s\n"
    "add z0.s, z0.s, z16.s\n"
    "ld1b { z9.b }, p0/Z, [x24, x14]\n"
    "add z31.s, z31.s, z16.s\n"
    "add z30.s, z30.s, z16.s\n"
    "ldp x23, x22, [%x[inptrs], #0x60]\n"
    "ldp x21, x20, [%x[inptrs], #0x70]\n"
    "smin z1.s, p2/M, z1.s, z6.s\n"
    "smax z0.s, p2/M, z0.s, z7.s\n"
    "st1b { z1.s }, p1, [x12, x28]\n"
    "ld1b { z2.b }, p0/Z, [x23, x14]\n"
    "smax z31.s, p2/M, z31.s, z7.s\n"
    "smax z30.s, p2/M, z30.s, z7.s\n"
    "ld1b { z23.b }, p0/Z, [x22, x14]\n"
    "ld1b { z22.b }, p0/Z, [x21, x14]\n"
    "ld1b { z5.b }, p0/Z, [x20, x14]\n"
    "zip2 z20.b, z15.b, z28.b\n"
    "zip1 z15.b, z15.b, z28.b\n"
    "smin z0.s, p2/M, z0.s, z6.s\n"
    "zip1 z19.b, z13.b, z29.b\n"
    "zip2 z29.b, z13.b, z29.b\n"
    "smin z31.s, p2/M, z31.s, z6.s\n"
    "smin z30.s, p2/M, z30.s, z6.s\n"
    "st1b { z0.s }, p1, [x11, x28]\n"
    "zip2 z13.b, z15.b, z19.b\n"
    "zip1 z15.b, z15.b, z19.b\n"
    "ldp x27, x26, [%x[inptrs], #0x0]\n"
    "st1b { z31.s }, p1, [x10, x28]\n"
    "zip1 z14.b, z20.b, z29.b\n"
    "zip2 z29.b, z20.b, z29.b\n"
    "ld1w { z10.s }, p2/Z, [%x[params], #4, MUL VL]\n"
    "st1b { z30.s }, p1, [x9, x28]\n"
    "zip2 z21.b, z9.b, z26.b\n"
    "zip1 z9.b, z9.b, z26.b\n"
    "incw x28\n"
    "zip1 z20.b, z27.b, z17.b\n"
    "zip2 z17.b, z27.b, z17.b\n"
    "ldp x25, x23, [%x[inptrs], #0x10]\n"
    "ldp x24, x22, [%x[inptrs], #0x20]\n"
    "zip2 z31.b, z18.b, z24.b\n"
    "zip1 z18.b, z18.b, z24.b\n"
    "ldp x21, x20, [%x[inptrs], #0x30]\n"
    "ld1b { z26.b }, p2/Z, [%x[params], #5, MUL VL]\n"
    "zip1 z27.b, z11.b, z4.b\n"
    "zip2 z4.b, z11.b, z4.b\n"
    "ld1b { z1.b }, p2/Z, [%x[params], #7, MUL VL]\n"
    "addvl %x[params], %x[params], #8\n"
    "zip2 z30.b, z2.b, z22.b\n"
    "zip1 z2.b, z2.b, z22.b\n"
    "zip1 z28.b, z23.b, z5.b\n"
    "zip2 z5.b, z23.b, z5.b\n"
    "zip2 z19.b, z9.b, z20.b\n"
    "zip1 z9.b, z9.b, z20.b\n"
    "zip1 z11.b, z21.b, z17.b\n"
    "zip2 z17.b, z21.b, z17.b\n"
    "zip2 z12.b, z18.b, z27.b\n"
    "zip1 z18.b, z18.b, z27.b\n"
    "zip1 z20.b, z31.b, z4.b\n"
    "zip2 z4.b, z31.b, z4.b\n"
    "zip2 z24.b, z2.b, z28.b\n"
    "zip1 z2.b, z2.b, z28.b\n"
    "zip1 z0.b, z30.b, z5.b\n"
    "zip2 z5.b, z30.b, z5.b\n"
    "mov z22.d, z10.d\n"
    "mov z31.d, z10.d\n"
    "mov z21.d, z10.d\n"
    "b.any 1b\n"
    : [params] "+&r" (params)
    : [inptrs] "r" (inptrs), [n_channels] "r" (n_channels), [offsetof_Requantize32_b_offset] "I" (offsetof(arm_gemm::Requantize32, b_offset)), [offsetof_Requantize32_c_offset] "I" (offsetof(arm_gemm::Requantize32, c_offset)), [offsetof_Requantize32_maxval] "I" (offsetof(arm_gemm::Requantize32, maxval)), [offsetof_Requantize32_minval] "I" (offsetof(arm_gemm::Requantize32, minval)), [outptrs] "r" (outptrs), [qp] "r" (&qp)
    : "cc", "memory", "p0", "p1", "p2", "x9", "x10", "x11", "x12", "x13", "x14", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
  );
}

}  // namespace depthwise
}  // namespace arm_conv

#endif  // defined(ARM_COMPUTE_ENABLE_SVE)
