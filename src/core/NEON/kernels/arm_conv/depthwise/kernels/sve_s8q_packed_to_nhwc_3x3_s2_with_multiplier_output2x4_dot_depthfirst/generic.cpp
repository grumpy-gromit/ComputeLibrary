/*
 * Copyright (c) 2021, 2023-2024 Arm Limited.
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


#include "arm_gemm.hpp"
#include <cstddef>
#include <cstdint>

#if defined(ARM_COMPUTE_ENABLE_SVE)

namespace arm_conv {
namespace depthwise {

void sve_s8q_packed_to_nhwc_3x3_s2_with_multiplier_output2x4_dot_depthfirst_impl(
  const int8_t *const *const inptrs,
  int8_t *const *const outptrs,
  const void *params,
  unsigned int n_output_channels,
  const arm_gemm::Requantize32& qp
)
{
  __asm__ __volatile__(
    "mov x20, #0x9\n"
    "whilelt p0.b, XZR, x20\n"
    "ldr x23, [%x[inptrs], #0x8]\n"
    "ldr x20, [%x[inptrs], #0x10]\n"
    "ldr x22, [%x[inptrs], #0x20]\n"
    "ldr x21, [%x[inptrs], #0x0]\n"
    "mov z13.b, #0x1\n"
    "lsr z13.s, z13.s, #0x8\n"
    "ld1b { z1.b }, p0/Z, [x23]\n"
    "ld1b { z2.b }, p0/Z, [x20]\n"
    "mov z8.d, z1.d\n"
    "mov z27.d, z1.d\n"
    "ldr x20, [%x[inptrs], #0x18]\n"
    "ld1b { z4.b }, p0/Z, [x22]\n"
    "mov z31.d, z1.d\n"
    "mov z28.d, z2.d\n"
    "ld1b { z0.b }, p0/Z, [x21]\n"
    "mov z30.d, z2.d\n"
    "mov z26.d, z2.d\n"
    "ld1b { z3.b }, p0/Z, [x20]\n"
    "mov z22.d, z4.d\n"
    "mov z10.d, z4.d\n"
    "ptrue p2.b\n"
    "ld1rw { z11.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_b_offset]]\n"
    "mov z18.d, z4.d\n"
    "ext z8.b, z8.b, z8.b, #0x2\n"
    "lsl x10, %x[n_channels], #0x2\n"
    "neg z11.s, p2/M, z11.s\n"
    "ext z27.b, z27.b, z27.b, #0x4\n"
    "ext z31.b, z31.b, z31.b, #0x6\n"
    "mov x9, #0x0\n"
    "whilelt p0.b, x9, x10\n"
    "ext z28.b, z28.b, z28.b, #0x2\n"
    "ext z30.b, z30.b, z30.b, #0x4\n"
    "ld1w { z14.s }, p0/Z, [%x[params]]\n"
    "mov x28, #0x0\n"
    "ext z26.b, z26.b, z26.b, #0x6\n"
    "ext z22.b, z22.b, z22.b, #0x2\n"
    "ldp x27, x26, [%x[outptrs], #0x0]\n"
    "ldp x25, x24, [%x[outptrs], #0x10]\n"
    "ext z10.b, z10.b, z10.b, #0x4\n"
    "ext z18.b, z18.b, z18.b, #0x6\n"
    "ldp x23, x22, [%x[outptrs], #0x20]\n"
    "ldp x21, x20, [%x[outptrs], #0x30]\n"
    "mov z21.d, z0.d\n"
    "mov z20.d, z0.d\n"
    "ld1rw { z9.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_c_offset]]\n"
    "ld1rw { z15.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_minval]]\n"
    "mov z19.d, z0.d\n"
    "mov z24.d, z3.d\n"
    "ld1rw { z12.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_maxval]]\n"
    "ld1b { z5.b }, p0/Z, [%x[params], #1, MUL VL]\n"
    "mov z17.d, z3.d\n"
    "mov z16.d, z3.d\n"
    "ld1b { z6.b }, p0/Z, [%x[params], #2, MUL VL]\n"
    "ld1b { z7.b }, p0/Z, [%x[params], #3, MUL VL]\n"
    "ext z21.b, z21.b, z21.b, #0x2\n"
    "ext z20.b, z20.b, z20.b, #0x4\n"
    "addvl %x[params], %x[params], #4\n"
    "ext z19.b, z19.b, z19.b, #0x6\n"
    "zip1 z1.s, z1.s, z27.s\n"
    "zip1 z8.s, z8.s, z31.s\n"
    "zip1 z2.s, z2.s, z30.s\n"
    "zip1 z28.s, z28.s, z26.s\n"
    "ext z24.b, z24.b, z24.b, #0x2\n"
    "ext z17.b, z17.b, z17.b, #0x4\n"
    "ext z16.b, z16.b, z16.b, #0x6\n"
    "zip1 z4.s, z4.s, z10.s\n"
    "zip1 z22.s, z22.s, z18.s\n"
    "zip1 z0.s, z0.s, z20.s\n"
    "zip1 z21.s, z21.s, z19.s\n"
    "zip1 z1.s, z1.s, z8.s\n"
    "zip1 z2.s, z2.s, z28.s\n"
    "zip1 z3.s, z3.s, z17.s\n"
    "zip1 z24.s, z24.s, z16.s\n"
    "zip1 z4.s, z4.s, z22.s\n"
    "zip1 z0.s, z0.s, z21.s\n"
    "mov z1.q, z1.q[0]\n"
    "mov z2.q, z2.q[0]\n"
    "zip1 z3.s, z3.s, z24.s\n"
    "mov z4.q, z4.q[0]\n"
    "mov z24.s, #0x0\n"
    "mov z25.s, #0x0\n"
    "sdot z24.s, z13.b, z1.b[0]\n"
    "mov z23.s, #0x0\n"
    "mov z22.s, #0x0\n"
    "sdot z25.s, z13.b, z1.b[1]\n"
    "mov z21.s, #0x0\n"
    "mov z19.s, #0x0\n"
    "sdot z23.s, z13.b, z1.b[2]\n"
    "mov z10.s, #0x0\n"
    "mov z8.s, #0x0\n"
    "sdot z22.s, z13.b, z1.b[3]\n"
    "mov z20.s, #0x0\n"
    "mov z18.s, #0x0\n"
    "sdot z21.s, z13.b, z2.b[0]\n"
    "mov z17.s, #0x0\n"
    "mov z16.s, #0x0\n"
    "sdot z19.s, z13.b, z2.b[1]\n"
    "sdot z10.s, z13.b, z2.b[2]\n"
    "sdot z8.s, z13.b, z2.b[3]\n"
    "mov z0.q, z0.q[0]\n"
    "sdot z20.s, z13.b, z4.b[0]\n"
    "sdot z18.s, z13.b, z4.b[1]\n"
    "mov z3.q, z3.q[0]\n"
    "sdot z17.s, z13.b, z4.b[2]\n"
    "sdot z16.s, z13.b, z4.b[3]\n"
    "mov z31.s, #0x0\n"
    "mov z30.s, #0x0\n"
    "mov z26.s, #0x0\n"
    "sdot z31.s, z13.b, z0.b[0]\n"
    "mov z27.s, #0x0\n"
    "mov z28.s, #0x0\n"
    "sdot z30.s, z13.b, z0.b[1]\n"
    "mov z29.s, #0x0\n"
    "sdot z26.s, z13.b, z0.b[2]\n"
    "sdot z27.s, z13.b, z0.b[3]\n"
    "sdot z28.s, z13.b, z3.b[0]\n"
    "sdot z29.s, z13.b, z3.b[1]\n"
    "add z24.s, z24.s, z21.s\n"
    "add z25.s, z25.s, z19.s\n"
    "add z23.s, z23.s, z10.s\n"
    "add z22.s, z22.s, z8.s\n"
    "add z21.s, z20.s, z21.s\n"
    "mov z20.s, #0x0\n"
    "sdot z20.s, z13.b, z3.b[2]\n"
    "add z19.s, z18.s, z19.s\n"
    "mov z18.s, #0x0\n"
    "sdot z18.s, z13.b, z3.b[3]\n"
    "add z17.s, z17.s, z10.s\n"
    "add z16.s, z16.s, z8.s\n"
    "add z24.s, z24.s, z31.s\n"
    "add z25.s, z25.s, z30.s\n"
    "mul z24.s, p2/M, z24.s, z11.s\n"
    "mul z25.s, p2/M, z25.s, z11.s\n"
    "add z26.s, z23.s, z26.s\n"
    "add z27.s, z22.s, z27.s\n"
    "mul z26.s, p2/M, z26.s, z11.s\n"
    "mul z27.s, p2/M, z27.s, z11.s\n"
    "add z28.s, z21.s, z28.s\n"
    "add z29.s, z19.s, z29.s\n"
    "mul z28.s, p2/M, z28.s, z11.s\n"
    "mul z29.s, p2/M, z29.s, z11.s\n"
    "add z30.s, z17.s, z20.s\n"
    "add z31.s, z16.s, z18.s\n"
    "mul z30.s, p2/M, z30.s, z11.s\n"
    "mul z31.s, p2/M, z31.s, z11.s\n"
    "zip1 z19.s, z24.s, z26.s\n"
    "zip1 z18.s, z25.s, z27.s\n"
    "zip1 z17.s, z28.s, z30.s\n"
    "zip1 z16.s, z29.s, z31.s\n"
    "zip1 z22.s, z19.s, z18.s\n"
    "zip1 z23.s, z17.s, z16.s\n"
    "add z24.s, z24.s, z14.s\n"
    "add z25.s, z25.s, z14.s\n"
    "add z26.s, z26.s, z14.s\n"
    "add z27.s, z27.s, z14.s\n"
    "add z28.s, z28.s, z14.s\n"
    "add z29.s, z29.s, z14.s\n"
    "add z30.s, z30.s, z14.s\n"
    "add z31.s, z31.s, z14.s\n"
    "1:"  // Loop
    "sdot z24.s, z5.b, z0.b[0]\n"
    "sdot z25.s, z5.b, z0.b[1]\n"
    "ld1w { z8.s }, p2/Z, [%x[params]]\n"
    "ld1w { z21.s }, p2/Z, [%x[params], #1, MUL VL]\n"
    "sdot z26.s, z5.b, z0.b[2]\n"
    "sdot z27.s, z5.b, z0.b[3]\n"
    "incb x9\n"
    "whilelt p1.s, x28, %x[n_channels]\n"
    "sdot z24.s, z6.b, z1.b[0]\n"
    "sdot z25.s, z6.b, z1.b[1]\n"
    "whilelt p0.b, x9, x10\n"
    "ld1w { z20.s }, p0/Z, [%x[params], #2, MUL VL]\n"
    "sdot z26.s, z6.b, z1.b[2]\n"
    "sdot z27.s, z6.b, z1.b[3]\n"
    "sdot z28.s, z5.b, z2.b[0]\n"
    "sdot z29.s, z5.b, z2.b[1]\n"
    "sdot z30.s, z5.b, z2.b[2]\n"
    "sdot z31.s, z5.b, z2.b[3]\n"
    "ld1b { z5.b }, p0/Z, [%x[params], #3, MUL VL]\n"
    "sdot z24.s, z7.b, z2.b[0]\n"
    "sdot z25.s, z7.b, z2.b[1]\n"
    ".inst 0x04a87718  // sqrdmulh z24.s, z24.s, z8.s\n"
    "sdot z26.s, z7.b, z2.b[2]\n"
    "sdot z27.s, z7.b, z2.b[3]\n"
    ".inst 0x04a87739  // sqrdmulh z25.s, z25.s, z8.s\n"
    "sdot z28.s, z6.b, z3.b[0]\n"
    "sdot z29.s, z6.b, z3.b[1]\n"
    ".inst 0x04a8775a  // sqrdmulh z26.s, z26.s, z8.s\n"
    "sdot z30.s, z6.b, z3.b[2]\n"
    "sdot z31.s, z6.b, z3.b[3]\n"
    ".inst 0x04a8777b  // sqrdmulh z27.s, z27.s, z8.s\n"
    "ld1b { z6.b }, p0/Z, [%x[params], #4, MUL VL]\n"
    "sdot z28.s, z7.b, z4.b[0]\n"
    "sdot z29.s, z7.b, z4.b[1]\n"
    "and z19.d, z24.d, z21.d\n"
    "sdot z30.s, z7.b, z4.b[2]\n"
    "sdot z31.s, z7.b, z4.b[3]\n"
    "and z18.d, z25.d, z21.d\n"
    "ld1b { z7.b }, p0/Z, [%x[params], #5, MUL VL]\n"
    "and z17.d, z26.d, z21.d\n"
    "and z16.d, z27.d, z21.d\n"
    "addvl %x[params], %x[params], #6\n"
    "asr z19.s, z19.s, #0x1f\n"
    "asr z18.s, z18.s, #0x1f\n"
    "asr z17.s, z17.s, #0x1f\n"
    "asr z16.s, z16.s, #0x1f\n"
    ".inst 0x04a8779c  // sqrdmulh z28.s, z28.s, z8.s\n"
    ".inst 0x04a877bd  // sqrdmulh z29.s, z29.s, z8.s\n"
    ".inst 0x04a877de  // sqrdmulh z30.s, z30.s, z8.s\n"
    ".inst 0x04a877ff  // sqrdmulh z31.s, z31.s, z8.s\n"
    "sqadd z24.s, z24.s, z19.s\n"
    "sqadd z25.s, z25.s, z18.s\n"
    ".inst 0x44828ab8  // srshl z24.s, p2/M, z24.s, z21.s\n"
    ".inst 0x44828ab9  // srshl z25.s, p2/M, z25.s, z21.s\n"
    "sqadd z26.s, z26.s, z17.s\n"
    "sqadd z27.s, z27.s, z16.s\n"
    ".inst 0x44828aba  // srshl z26.s, p2/M, z26.s, z21.s\n"
    ".inst 0x44828abb  // srshl z27.s, p2/M, z27.s, z21.s\n"
    "and z19.d, z28.d, z21.d\n"
    "and z18.d, z29.d, z21.d\n"
    "and z17.d, z30.d, z21.d\n"
    "and z16.d, z31.d, z21.d\n"
    "asr z19.s, z19.s, #0x1f\n"
    "asr z18.s, z18.s, #0x1f\n"
    "asr z17.s, z17.s, #0x1f\n"
    "asr z16.s, z16.s, #0x1f\n"
    "sqadd z28.s, z28.s, z19.s\n"
    "sqadd z29.s, z29.s, z18.s\n"
    ".inst 0x44828abc  // srshl z28.s, p2/M, z28.s, z21.s\n"
    ".inst 0x44828abd  // srshl z29.s, p2/M, z29.s, z21.s\n"
    "sqadd z30.s, z30.s, z17.s\n"
    "sqadd z31.s, z31.s, z16.s\n"
    ".inst 0x44828abe  // srshl z30.s, p2/M, z30.s, z21.s\n"
    ".inst 0x44828abf  // srshl z31.s, p2/M, z31.s, z21.s\n"
    "add z24.s, z24.s, z9.s\n"
    "add z25.s, z25.s, z9.s\n"
    "smin z24.s, p2/M, z24.s, z12.s\n"
    "smin z25.s, p2/M, z25.s, z12.s\n"
    "add z26.s, z26.s, z9.s\n"
    "add z27.s, z27.s, z9.s\n"
    "smin z26.s, p2/M, z26.s, z12.s\n"
    "smin z27.s, p2/M, z27.s, z12.s\n"
    "add z28.s, z28.s, z9.s\n"
    "add z29.s, z29.s, z9.s\n"
    "smin z28.s, p2/M, z28.s, z12.s\n"
    "smin z29.s, p2/M, z29.s, z12.s\n"
    "add z30.s, z30.s, z9.s\n"
    "add z31.s, z31.s, z9.s\n"
    "smin z30.s, p2/M, z30.s, z12.s\n"
    "smin z31.s, p2/M, z31.s, z12.s\n"
    "smax z24.s, p2/M, z24.s, z15.s\n"
    "smax z25.s, p2/M, z25.s, z15.s\n"
    "st1b { z24.s }, p1, [x27, x28]\n"
    "mov z24.s, z22.s[0]\n"
    "smax z26.s, p2/M, z26.s, z15.s\n"
    "smax z27.s, p2/M, z27.s, z15.s\n"
    "st1b { z25.s }, p1, [x26, x28]\n"
    "mov z25.s, z22.s[1]\n"
    "smax z28.s, p2/M, z28.s, z15.s\n"
    "smax z29.s, p2/M, z29.s, z15.s\n"
    "st1b { z26.s }, p1, [x25, x28]\n"
    "mov z26.s, z22.s[2]\n"
    "smax z30.s, p2/M, z30.s, z15.s\n"
    "smax z31.s, p2/M, z31.s, z15.s\n"
    "st1b { z27.s }, p1, [x24, x28]\n"
    "mov z27.s, z22.s[3]\n"
    "st1b { z28.s }, p1, [x23, x28]\n"
    "mov z28.s, z23.s[0]\n"
    "add z24.s, z24.s, z20.s\n"
    "st1b { z29.s }, p1, [x22, x28]\n"
    "mov z29.s, z23.s[1]\n"
    "add z25.s, z25.s, z20.s\n"
    "st1b { z30.s }, p1, [x21, x28]\n"
    "mov z30.s, z23.s[2]\n"
    "add z26.s, z26.s, z20.s\n"
    "st1b { z31.s }, p1, [x20, x28]\n"
    "mov z31.s, z23.s[3]\n"
    "incw x28\n"
    "add z27.s, z27.s, z20.s\n"
    "add z28.s, z28.s, z20.s\n"
    "add z29.s, z29.s, z20.s\n"
    "add z30.s, z30.s, z20.s\n"
    "add z31.s, z31.s, z20.s\n"
    "b.any 1b\n"
    : [params] "+&r" (params)
    : [inptrs] "r" (inptrs), [n_channels] "r" (n_output_channels), [offsetof_Requantize32_b_offset] "I" (offsetof(arm_gemm::Requantize32, b_offset)), [offsetof_Requantize32_c_offset] "I" (offsetof(arm_gemm::Requantize32, c_offset)), [offsetof_Requantize32_maxval] "I" (offsetof(arm_gemm::Requantize32, maxval)), [offsetof_Requantize32_minval] "I" (offsetof(arm_gemm::Requantize32, minval)), [outptrs] "r" (outptrs), [qp] "r" (&qp)
    : "cc", "memory", "p0", "p1", "p2", "x9", "x10", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
  );
}

}  // namespace depthwise
}  // namespace arm_conv

#endif  // defined(ARM_COMPUTE_ENABLE_SVE)
