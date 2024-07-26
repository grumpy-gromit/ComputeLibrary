/*
 * Copyright (c) 2019-2021, 2023-2024 Arm Limited.
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
#ifdef ARM_COMPUTE_ENABLE_SVE

#include <cstddef>
#include <cstdint>

namespace arm_gemm {

void sve_interleaved_u8u32_mmla_8x3VL(
    const uint8_t *Apanel,
    const uint8_t *Bpanel,
    uint32_t *Cpanel,
    int ablocks,
    int bblocks,
    int K) {

    struct KernelArgs {
        size_t K = {};
        const uint8_t *Bpanel = {};
        size_t bblocks = {};
    } ka;

    ka.K = (K/8) - 1;
    ka.Bpanel = Bpanel;
    ka.bblocks = bblocks;

    __asm__ __volatile__(
      "ptrue p0.b\n"
      "1:"  // Height loop
      "ldr x23, [%x[args_ptr], %[offsetof_bblocks]]\n"
      "ldr x22, [%x[args_ptr], %[offsetof_Bpanel]]\n"
      "mov x21, %x[Apanel]\n"
      "2:"  // Width loop
      "ldr x20, [%x[args_ptr], %[offsetof_K]]\n"
      "mov %x[Apanel], x21\n"
      "cmp x20, #0x2\n"
      "mov z8.s, #0x0\n"
      "mov z9.s, #0x0\n"
      "ld1b { z4.b }, p0/Z, [x22]\n"
      "mov z10.s, #0x0\n"
      "mov z11.s, #0x0\n"
      "ld1rqb { z0.b }, p0/Z, [%x[Apanel]]\n"
      "mov z12.s, #0x0\n"
      "mov z13.s, #0x0\n"
      "ld1rqb { z1.b }, p0/Z, [%x[Apanel], #16]\n"
      "mov z14.s, #0x0\n"
      "mov z15.s, #0x0\n"
      "ld1b { z5.b }, p0/Z, [x22, #1, MUL VL]\n"
      "mov z16.s, #0x0\n"
      "mov z17.s, #0x0\n"
      "ld1rqb { z2.b }, p0/Z, [%x[Apanel], #32]\n"
      "mov z18.s, #0x0\n"
      "mov z19.s, #0x0\n"
      "addvl x22, x22, #2\n"
      "mov z20.s, #0x0\n"
      "mov z21.s, #0x0\n"
      "add %x[Apanel], %x[Apanel], #0x30\n"
      "mov z22.s, #0x0\n"
      "mov z23.s, #0x0\n"
      "mov z24.s, #0x0\n"
      "mov z25.s, #0x0\n"
      "mov z26.s, #0x0\n"
      "mov z27.s, #0x0\n"
      "mov z28.s, #0x0\n"
      "mov z29.s, #0x0\n"
      "mov z30.s, #0x0\n"
      "mov z31.s, #0x0\n"
      "blt 4f\n"
      "3:"  // main loop head
      "ld1rqb { z6.b }, p0/Z, [%x[Apanel]]\n"
      ".inst 0x45c49808  // ummla z8.s, z0.b, z4.b\n"
      ".inst 0x45c5980b  // ummla z11.s, z0.b, z5.b\n"
      ".inst 0x45c4982e  // ummla z14.s, z1.b, z4.b\n"
      ".inst 0x45c59831  // ummla z17.s, z1.b, z5.b\n"
      "ld1b { z7.b }, p0/Z, [x22]\n"
      ".inst 0x45c49854  // ummla z20.s, z2.b, z4.b\n"
      ".inst 0x45c59857  // ummla z23.s, z2.b, z5.b\n"
      "ld1b { z3.b }, p0/Z, [x22, #1, MUL VL]\n"
      ".inst 0x45c498da  // ummla z26.s, z6.b, z4.b\n"
      ".inst 0x45c598dd  // ummla z29.s, z6.b, z5.b\n"
      "ld1b { z5.b }, p0/Z, [x22, #2, MUL VL]\n"
      "ld1b { z4.b }, p0/Z, [x22, #3, MUL VL]\n"
      ".inst 0x45c79809  // ummla z9.s, z0.b, z7.b\n"
      ".inst 0x45c3980c  // ummla z12.s, z0.b, z3.b\n"
      ".inst 0x45c7982f  // ummla z15.s, z1.b, z7.b\n"
      ".inst 0x45c39832  // ummla z18.s, z1.b, z3.b\n"
      "sub x20, x20, #0x2\n"
      ".inst 0x45c79855  // ummla z21.s, z2.b, z7.b\n"
      ".inst 0x45c39858  // ummla z24.s, z2.b, z3.b\n"
      "cmp x20, #0x2\n"
      ".inst 0x45c798db  // ummla z27.s, z6.b, z7.b\n"
      ".inst 0x45c398de  // ummla z30.s, z6.b, z3.b\n"
      "ld1b { z3.b }, p0/Z, [x22, #4, MUL VL]\n"
      ".inst 0x45c5980a  // ummla z10.s, z0.b, z5.b\n"
      ".inst 0x45c4980d  // ummla z13.s, z0.b, z4.b\n"
      "ld1rqb { z0.b }, p0/Z, [%x[Apanel], #16]\n"
      ".inst 0x45c59830  // ummla z16.s, z1.b, z5.b\n"
      ".inst 0x45c49833  // ummla z19.s, z1.b, z4.b\n"
      "ld1rqb { z1.b }, p0/Z, [%x[Apanel], #32]\n"
      ".inst 0x45c59856  // ummla z22.s, z2.b, z5.b\n"
      ".inst 0x45c49859  // ummla z25.s, z2.b, z4.b\n"
      "ld1b { z7.b }, p0/Z, [x22, #5, MUL VL]\n"
      ".inst 0x45c598dc  // ummla z28.s, z6.b, z5.b\n"
      ".inst 0x45c498df  // ummla z31.s, z6.b, z4.b\n"
      "ld1rqb { z5.b }, p0/Z, [%x[Apanel], #48]\n"
      "ld1rqb { z6.b }, p0/Z, [%x[Apanel], #64]\n"
      "ld1b { z2.b }, p0/Z, [x22, #6, MUL VL]\n"
      ".inst 0x45c39808  // ummla z8.s, z0.b, z3.b\n"
      "ld1b { z4.b }, p0/Z, [x22, #7, MUL VL]\n"
      "addvl x22, x22, #16\n"
      ".inst 0x45c7980b  // ummla z11.s, z0.b, z7.b\n"
      ".inst 0x45c3982e  // ummla z14.s, z1.b, z3.b\n"
      ".inst 0x45c79831  // ummla z17.s, z1.b, z7.b\n"
      ".inst 0x45c398b4  // ummla z20.s, z5.b, z3.b\n"
      ".inst 0x45c798b7  // ummla z23.s, z5.b, z7.b\n"
      ".inst 0x45c398da  // ummla z26.s, z6.b, z3.b\n"
      ".inst 0x45c798dd  // ummla z29.s, z6.b, z7.b\n"
      "ld1b { z3.b }, p0/Z, [x22, #-8, MUL VL]\n"
      "ld1b { z7.b }, p0/Z, [x22, #-7, MUL VL]\n"
      ".inst 0x45c29809  // ummla z9.s, z0.b, z2.b\n"
      ".inst 0x45c4980c  // ummla z12.s, z0.b, z4.b\n"
      ".inst 0x45c2982f  // ummla z15.s, z1.b, z2.b\n"
      ".inst 0x45c49832  // ummla z18.s, z1.b, z4.b\n"
      ".inst 0x45c298b5  // ummla z21.s, z5.b, z2.b\n"
      ".inst 0x45c498b8  // ummla z24.s, z5.b, z4.b\n"
      ".inst 0x45c298db  // ummla z27.s, z6.b, z2.b\n"
      ".inst 0x45c498de  // ummla z30.s, z6.b, z4.b\n"
      "ld1b { z4.b }, p0/Z, [x22, #-6, MUL VL]\n"
      ".inst 0x45c3980a  // ummla z10.s, z0.b, z3.b\n"
      ".inst 0x45c7980d  // ummla z13.s, z0.b, z7.b\n"
      "ld1rqb { z0.b }, p0/Z, [%x[Apanel], #80]\n"
      ".inst 0x45c39830  // ummla z16.s, z1.b, z3.b\n"
      ".inst 0x45c79833  // ummla z19.s, z1.b, z7.b\n"
      "ld1rqb { z1.b }, p0/Z, [%x[Apanel], #96]\n"
      ".inst 0x45c398b6  // ummla z22.s, z5.b, z3.b\n"
      ".inst 0x45c798b9  // ummla z25.s, z5.b, z7.b\n"
      "ld1b { z5.b }, p0/Z, [x22, #-5, MUL VL]\n"
      ".inst 0x45c398dc  // ummla z28.s, z6.b, z3.b\n"
      ".inst 0x45c798df  // ummla z31.s, z6.b, z7.b\n"
      "ld1rqb { z2.b }, p0/Z, [%x[Apanel], #112]\n"
      "add %x[Apanel], %x[Apanel], #0x80\n"
      "addvl x22, x22, #-4\n"
      "bge 3b\n"
      "4:"  // main loop skip
      "ld1rqb { z7.b }, p0/Z, [%x[Apanel]]\n"
      ".inst 0x45c49808  // ummla z8.s, z0.b, z4.b\n"
      ".inst 0x45c5980b  // ummla z11.s, z0.b, z5.b\n"
      ".inst 0x45c4982e  // ummla z14.s, z1.b, z4.b\n"
      ".inst 0x45c59831  // ummla z17.s, z1.b, z5.b\n"
      "ld1b { z6.b }, p0/Z, [x22]\n"
      ".inst 0x45c49854  // ummla z20.s, z2.b, z4.b\n"
      ".inst 0x45c59857  // ummla z23.s, z2.b, z5.b\n"
      "ld1b { z3.b }, p0/Z, [x22, #1, MUL VL]\n"
      ".inst 0x45c498fa  // ummla z26.s, z7.b, z4.b\n"
      ".inst 0x45c598fd  // ummla z29.s, z7.b, z5.b\n"
      "ld1b { z5.b }, p0/Z, [x22, #2, MUL VL]\n"
      "ld1b { z4.b }, p0/Z, [x22, #3, MUL VL]\n"
      ".inst 0x45c69809  // ummla z9.s, z0.b, z6.b\n"
      ".inst 0x45c3980c  // ummla z12.s, z0.b, z3.b\n"
      ".inst 0x45c6982f  // ummla z15.s, z1.b, z6.b\n"
      ".inst 0x45c39832  // ummla z18.s, z1.b, z3.b\n"
      "add %x[Apanel], %x[Apanel], #0x10\n"
      ".inst 0x45c69855  // ummla z21.s, z2.b, z6.b\n"
      ".inst 0x45c39858  // ummla z24.s, z2.b, z3.b\n"
      "addvl x22, x22, #4\n"
      ".inst 0x45c698fb  // ummla z27.s, z7.b, z6.b\n"
      ".inst 0x45c398fe  // ummla z30.s, z7.b, z3.b\n"
      ".inst 0x45c5980a  // ummla z10.s, z0.b, z5.b\n"
      ".inst 0x45c4980d  // ummla z13.s, z0.b, z4.b\n"
      ".inst 0x45c59830  // ummla z16.s, z1.b, z5.b\n"
      ".inst 0x45c49833  // ummla z19.s, z1.b, z4.b\n"
      ".inst 0x45c59856  // ummla z22.s, z2.b, z5.b\n"
      ".inst 0x45c49859  // ummla z25.s, z2.b, z4.b\n"
      ".inst 0x45c598fc  // ummla z28.s, z7.b, z5.b\n"
      ".inst 0x45c498ff  // ummla z31.s, z7.b, z4.b\n"
      "cbz x20, 5f\n"
      "ld1b { z1.b }, p0/Z, [x22]\n"
      "ld1rqb { z7.b }, p0/Z, [%x[Apanel]]\n"
      ".inst 0x45c198e8  // ummla z8.s, z7.b, z1.b\n"
      "ld1rqb { z6.b }, p0/Z, [%x[Apanel], #16]\n"
      "ld1b { z0.b }, p0/Z, [x22, #1, MUL VL]\n"
      ".inst 0x45c098eb  // ummla z11.s, z7.b, z0.b\n"
      "ld1rqb { z5.b }, p0/Z, [%x[Apanel], #32]\n"
      "ld1rqb { z4.b }, p0/Z, [%x[Apanel], #48]\n"
      ".inst 0x45c198ce  // ummla z14.s, z6.b, z1.b\n"
      ".inst 0x45c098d1  // ummla z17.s, z6.b, z0.b\n"
      ".inst 0x45c198b4  // ummla z20.s, z5.b, z1.b\n"
      "ld1b { z3.b }, p0/Z, [x22, #2, MUL VL]\n"
      ".inst 0x45c098b7  // ummla z23.s, z5.b, z0.b\n"
      ".inst 0x45c1989a  // ummla z26.s, z4.b, z1.b\n"
      "ld1b { z2.b }, p0/Z, [x22, #3, MUL VL]\n"
      ".inst 0x45c0989d  // ummla z29.s, z4.b, z0.b\n"
      "ld1b { z1.b }, p0/Z, [x22, #4, MUL VL]\n"
      "ld1b { z0.b }, p0/Z, [x22, #5, MUL VL]\n"
      ".inst 0x45c398e9  // ummla z9.s, z7.b, z3.b\n"
      ".inst 0x45c298ec  // ummla z12.s, z7.b, z2.b\n"
      "addvl x22, x22, #6\n"
      ".inst 0x45c398cf  // ummla z15.s, z6.b, z3.b\n"
      ".inst 0x45c298d2  // ummla z18.s, z6.b, z2.b\n"
      "add %x[Apanel], %x[Apanel], #0x40\n"
      ".inst 0x45c398b5  // ummla z21.s, z5.b, z3.b\n"
      ".inst 0x45c298b8  // ummla z24.s, z5.b, z2.b\n"
      ".inst 0x45c3989b  // ummla z27.s, z4.b, z3.b\n"
      ".inst 0x45c2989e  // ummla z30.s, z4.b, z2.b\n"
      ".inst 0x45c198ea  // ummla z10.s, z7.b, z1.b\n"
      ".inst 0x45c098ed  // ummla z13.s, z7.b, z0.b\n"
      ".inst 0x45c198d0  // ummla z16.s, z6.b, z1.b\n"
      ".inst 0x45c098d3  // ummla z19.s, z6.b, z0.b\n"
      ".inst 0x45c198b6  // ummla z22.s, z5.b, z1.b\n"
      ".inst 0x45c098b9  // ummla z25.s, z5.b, z0.b\n"
      ".inst 0x45c1989c  // ummla z28.s, z4.b, z1.b\n"
      ".inst 0x45c0989f  // ummla z31.s, z4.b, z0.b\n"
      "5:"  // multiply loop done
      "uzp1 z0.d, z8.d, z11.d\n"
      "uzp2 z8.d, z8.d, z11.d\n"
      "st1w { z0.s }, p0, [%x[Cpanel]]\n"
      "uzp1 z0.d, z9.d, z12.d\n"
      "uzp2 z9.d, z9.d, z12.d\n"
      "st1w { z0.s }, p0, [%x[Cpanel], #1, MUL VL]\n"
      "uzp1 z0.d, z10.d, z13.d\n"
      "uzp2 z10.d, z10.d, z13.d\n"
      "st1w { z0.s }, p0, [%x[Cpanel], #2, MUL VL]\n"
      "st1w { z8.s }, p0, [%x[Cpanel], #3, MUL VL]\n"
      "uzp1 z0.d, z14.d, z17.d\n"
      "uzp2 z14.d, z14.d, z17.d\n"
      "st1w { z9.s }, p0, [%x[Cpanel], #4, MUL VL]\n"
      "uzp1 z1.d, z15.d, z18.d\n"
      "subs x23, x23, #0x1\n"
      "st1w { z10.s }, p0, [%x[Cpanel], #5, MUL VL]\n"
      "uzp2 z15.d, z15.d, z18.d\n"
      "uzp1 z17.d, z16.d, z19.d\n"
      "st1w { z0.s }, p0, [%x[Cpanel], #6, MUL VL]\n"
      "uzp2 z16.d, z16.d, z19.d\n"
      "uzp1 z0.d, z20.d, z23.d\n"
      "st1w { z1.s }, p0, [%x[Cpanel], #7, MUL VL]\n"
      "addvl %x[Cpanel], %x[Cpanel], #16\n"
      "uzp2 z20.d, z20.d, z23.d\n"
      "st1w { z17.s }, p0, [%x[Cpanel], #-8, MUL VL]\n"
      "uzp1 z23.d, z21.d, z24.d\n"
      "uzp2 z21.d, z21.d, z24.d\n"
      "st1w { z14.s }, p0, [%x[Cpanel], #-7, MUL VL]\n"
      "uzp1 z19.d, z22.d, z25.d\n"
      "uzp2 z22.d, z22.d, z25.d\n"
      "st1w { z15.s }, p0, [%x[Cpanel], #-6, MUL VL]\n"
      "uzp1 z18.d, z26.d, z29.d\n"
      "uzp2 z26.d, z26.d, z29.d\n"
      "st1w { z16.s }, p0, [%x[Cpanel], #-5, MUL VL]\n"
      "uzp1 z17.d, z27.d, z30.d\n"
      "uzp2 z27.d, z27.d, z30.d\n"
      "st1w { z0.s }, p0, [%x[Cpanel], #-4, MUL VL]\n"
      "uzp1 z16.d, z28.d, z31.d\n"
      "uzp2 z28.d, z28.d, z31.d\n"
      "st1w { z23.s }, p0, [%x[Cpanel], #-3, MUL VL]\n"
      "st1w { z19.s }, p0, [%x[Cpanel], #-2, MUL VL]\n"
      "st1w { z20.s }, p0, [%x[Cpanel], #-1, MUL VL]\n"
      "st1w { z21.s }, p0, [%x[Cpanel]]\n"
      "st1w { z22.s }, p0, [%x[Cpanel], #1, MUL VL]\n"
      "st1w { z18.s }, p0, [%x[Cpanel], #2, MUL VL]\n"
      "st1w { z17.s }, p0, [%x[Cpanel], #3, MUL VL]\n"
      "st1w { z16.s }, p0, [%x[Cpanel], #4, MUL VL]\n"
      "st1w { z26.s }, p0, [%x[Cpanel], #5, MUL VL]\n"
      "st1w { z27.s }, p0, [%x[Cpanel], #6, MUL VL]\n"
      "st1w { z28.s }, p0, [%x[Cpanel], #7, MUL VL]\n"
      "addvl %x[Cpanel], %x[Cpanel], #8\n"
      "bgt 2b\n"
      "subs %x[ablocks], %x[ablocks], #0x1\n"
      "bne 1b\n"
      : [Apanel] "+&r" (Apanel), [Cpanel] "+&r" (Cpanel), [ablocks] "+&r" (ablocks)
      : [args_ptr] "r" (&ka), [offsetof_Bpanel] "I" (offsetof(KernelArgs, Bpanel)), [offsetof_K] "I" (offsetof(KernelArgs, K)), [offsetof_bblocks] "I" (offsetof(KernelArgs, bblocks))
      : "cc", "memory", "p0", "x20", "x21", "x22", "x23", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
    );
}

} // namespace arm_gemm
#endif // ARM_COMPUTE_ENABLE_SVE
