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

#if defined(ARM_COMPUTE_ENABLE_SME)

template <>
void interleave_block<2, 4, VLType::SME, true>(
  uint8_t * &out, const uint8_t * const *in,
  size_t width, size_t height, size_t row_offset, bool first
)
{
  __asm__ __volatile__(
      ".inst 0xd503477f  // SMSTART ZA\n"
      "cntb x21\n"
      "mov x23, %x[width]\n"
      "mov z20.b, #0x1\n"
      "incb x23\n"
      "mov x20, %x[width]\n"
      "mov z19.s, #0x0\n"
      "mov z18.s, #0x0\n"
      "sub x17, x21, #0x1\n"
      "cntw x16\n"
      "sub x23, x23, #0x1\n"
      "ands x17, x20, x17\n"
      "udiv x23, x23, x21\n"  // n_passes = ceildiv(width, VL<T>)
      "csel x17, x17, x21, NE\n"
      "lsl x22, %x[height], #0x1\n"  // height * 2
      "lsl x21, x16, #0x1\n"
      "sub x20, x23, #0x1\n"
      "add x17, x17, #0x3\n"
      "whilelt p9.b, XZR, x22\n"
      "whilelt p8.b, x21, x22\n"
      "mov x15, #0x0\n"
      "cntw x14, ALL, MUL #2\n"
      "cntw x11, ALL, MUL #3\n"
      "ptrue p4.b\n"
      "lsr x20, x20, #0x1\n"  // n_loops = (n_passes - 1) / 2
      "and x10, x23, #0x1\n"  // odd_tail = bool(n_passes & 0x1)
      "lsr x17, x17, #0x2\n"
      "sub x9, x16, #0x2\n"
      "ptrue p11.s\n"
      "zip1 p10.b, p9.b, p8.b\n"
      "mov x28, %x[row_offset]\n"
      "mov x27, %x[out]\n"
      "whilelt p9.b, x15, %x[width]\n"
      "whilelt p8.b, x15, %x[width]\n"
      "cbnz %x[first], 1f\n"
      "addvl x27, x27, #-2\n"
      "ld1w { z19.s }, p4/Z, [x27]\n"
      "ld1w { z18.s }, p4/Z, [x27, #1, MUL VL]\n"
      "1:"  // K loop: Load row sums: End
      "mov x26, %x[in]\n"
      "add x25, %x[in], x16, LSL #3\n"
      "ldr x24, [x26, #0x0]\n"
      "ldr x23, [x25, #0x0]\n"
      "mov x12, #0x0\n"
      "ldr x22, [x26, #0x8]\n"
      "add x26, x26, #0x10\n"
      "ldr x21, [x25, #0x8]\n"
      "add x25, x25, #0x10\n"
      "cbz x9, 3f\n"
      "2:"  // K loop: Charge: Loop
      ".inst 0x25246140  // psel p0.b, p8.b/Z, p10.b[w12]\n"
      ".inst 0xe01c0300  // ld1b { za0h.b[x12] }, p0/Z, [x24, x28]\n"
      ".inst 0x252c6140  // psel p0.b, p8.b/Z, p10.b[w12, #1]\n"
      "ldr x24, [x26, #0x0]\n"
      ".inst 0xe01c02e1  // ld1b { za0h.b[x12, #1] }, p0/Z, [x23, x28]\n"
      ".inst 0x25646141  // psel p1.b, p8.b/Z, p10.b[w12, #4]\n"
      ".inst 0x256c6140  // psel p0.b, p8.b/Z, p10.b[w12, #5]\n"
      "ldr x23, [x25, #0x0]\n"
      ".inst 0xe01c06c4  // ld1b { za0h.b[x12, #4] }, p1/Z, [x22, x28]\n"
      "ldr x22, [x26, #0x8]\n"
      "add x26, x26, #0x10\n"
      ".inst 0xe01c02a5  // ld1b { za0h.b[x12, #5] }, p0/Z, [x21, x28]\n"
      "add x12, x12, #0x8\n"
      "cmp x12, x9, LSL #2\n"
      "ldr x21, [x25, #0x8]\n"
      "add x25, x25, #0x10\n"
      "blt 2b\n"
      "3:"  // K loop: Charge: End
      ".inst 0x25246140  // psel p0.b, p8.b/Z, p10.b[w12]\n"
      ".inst 0xe01c0300  // ld1b { za0h.b[x12] }, p0/Z, [x24, x28]\n"
      ".inst 0x252c6140  // psel p0.b, p8.b/Z, p10.b[w12, #1]\n"
      ".inst 0xe01c02e1  // ld1b { za0h.b[x12, #1] }, p0/Z, [x23, x28]\n"
      ".inst 0x25646141  // psel p1.b, p8.b/Z, p10.b[w12, #4]\n"
      ".inst 0x256c6140  // psel p0.b, p8.b/Z, p10.b[w12, #5]\n"
      ".inst 0xe01c06c4  // ld1b { za0h.b[x12, #4] }, p1/Z, [x22, x28]\n"
      "mov x26, %x[in]\n"
      "add x25, %x[in], x16, LSL #3\n"
      "ldr x24, [x26, #0x0]\n"
      ".inst 0xe01c02a5  // ld1b { za0h.b[x12, #5] }, p0/Z, [x21, x28]\n"
      "ldr x23, [x25, #0x0]\n"
      "incb x28\n"
      "incb x15\n"
      "ldr x22, [x26, #0x8]\n"
      "add x26, x26, #0x10\n"
      "ldr x21, [x25, #0x8]\n"
      "add x25, x25, #0x10\n"
      "cbz x20, 9f\n"
      "mov x20, x20\n"
      "4:"  // K loop: Main loop
      "whilelt p8.b, x15, %x[width]\n"
      "mov x13, #0x0\n"
      "mov x12, #0x0\n"
      "cbz x9, 6f\n"
      "5:"  // K loop: Main loop: First: Loop
      ".inst 0x25356140  // psel p0.b, p8.b/Z, p10.b[w13, #2]\n"
      ".inst 0xe01c2302  // ld1b { za0h.b[x13, #2] }, p0/Z, [x24, x28]\n"
      ".inst 0x253d6140  // psel p0.b, p8.b/Z, p10.b[w13, #3]\n"
      "ldr x24, [x26, #0x0]\n"
      ".inst 0xe01c22e3  // ld1b { za0h.b[x13, #3] }, p0/Z, [x23, x28]\n"
      ".inst 0x25756140  // psel p0.b, p8.b/Z, p10.b[w13, #6]\n"
      ".inst 0x257d6142  // psel p2.b, p8.b/Z, p10.b[w13, #7]\n"
      "ldr x23, [x25, #0x0]\n"
      ".inst 0xe01c22c6  // ld1b { za0h.b[x13, #6] }, p0/Z, [x22, x28]\n"
      ".inst 0x25306d21  // psel p1.s, p11.s/Z, p9.s[w12]\n"
      "ldr x22, [x26, #0x8]\n"
      ".inst 0x25306d20  // psel p0.s, p11.s/Z, p9.s[w12]\n"
      ".inst 0xe01c2aa7  // ld1b { za0h.b[x13, #7] }, p2/Z, [x21, x28]\n"
      "ldr x21, [x25, #0x8]\n"
      ".inst 0xe0bf8760  // st1w { za0v.s[x12] }, p1/Z, [x27, XZR, LSL #2]\n"
      ".inst 0xc0829010  // mova z16.s, p4/M, za0v.s[x12]\n"
      ".inst 0xc0829091  // mova z17.s, p4/M, za1v.s[x12]\n"
      "udot z19.s, z16.b, z20.b\n"
      ".inst 0xe0b08364  // st1w { za1v.s[x12] }, p0/Z, [x27, x16, LSL #2]\n"
      ".inst 0x25706d20  // psel p0.s, p11.s/Z, p9.s[w12, #1]\n"
      "udot z18.s, z17.b, z20.b\n"
      ".inst 0xe0ae8361  // st1w { za0v.s[x12, #1] }, p0/Z, [x27, x14, LSL #2]\n"
      ".inst 0x25706d20  // psel p0.s, p11.s/Z, p9.s[w12, #1]\n"
      ".inst 0xc0829030  // mova z16.s, p4/M, za0v.s[x12, #1]\n"
      ".inst 0xe0ab8365  // st1w { za1v.s[x12, #1] }, p0/Z, [x27, x11, LSL #2]\n"
      ".inst 0xc08290b1  // mova z17.s, p4/M, za1v.s[x12, #1]\n"
      "add x12, x12, #0x2\n"
      "cmp x12, x9\n"
      "add x26, x26, #0x10\n"
      "add x25, x25, #0x10\n"
      "udot z19.s, z16.b, z20.b\n"
      "udot z18.s, z17.b, z20.b\n"
      "addvl x27, x27, #4\n"
      "add x13, x13, #0x8\n"
      "blt 5b\n"
      "6:"  // K loop: Main loop: First: Tail
      ".inst 0x25356140  // psel p0.b, p8.b/Z, p10.b[w13, #2]\n"
      ".inst 0xe01c2302  // ld1b { za0h.b[x13, #2] }, p0/Z, [x24, x28]\n"
      ".inst 0x253d6140  // psel p0.b, p8.b/Z, p10.b[w13, #3]\n"
      ".inst 0xe01c22e3  // ld1b { za0h.b[x13, #3] }, p0/Z, [x23, x28]\n"
      ".inst 0x25756141  // psel p1.b, p8.b/Z, p10.b[w13, #6]\n"
      ".inst 0x257d6140  // psel p0.b, p8.b/Z, p10.b[w13, #7]\n"
      ".inst 0xe01c26c6  // ld1b { za0h.b[x13, #6] }, p1/Z, [x22, x28]\n"
      "mov x26, %x[in]\n"
      "add x25, %x[in], x16, LSL #3\n"
      "ldr x24, [x26, #0x0]\n"
      ".inst 0xe01c22a7  // ld1b { za0h.b[x13, #7] }, p0/Z, [x21, x28]\n"
      ".inst 0xc0829010  // mova z16.s, p4/M, za0v.s[x12]\n"
      ".inst 0x25306d23  // psel p3.s, p11.s/Z, p9.s[w12]\n"
      "udot z19.s, z16.b, z20.b\n"
      ".inst 0xc0829091  // mova z17.s, p4/M, za1v.s[x12]\n"
      "udot z18.s, z17.b, z20.b\n"
      "ldr x23, [x25, #0x0]\n"
      ".inst 0x25306d22  // psel p2.s, p11.s/Z, p9.s[w12]\n"
      "ldr x22, [x26, #0x8]\n"
      ".inst 0x25706d21  // psel p1.s, p11.s/Z, p9.s[w12, #1]\n"
      ".inst 0xc0829030  // mova z16.s, p4/M, za0v.s[x12, #1]\n"
      ".inst 0x25706d20  // psel p0.s, p11.s/Z, p9.s[w12, #1]\n"
      "ldr x21, [x25, #0x8]\n"
      ".inst 0xe0bf8f60  // st1w { za0v.s[x12] }, p3/Z, [x27, XZR, LSL #2]\n"
      ".inst 0xc08290b1  // mova z17.s, p4/M, za1v.s[x12, #1]\n"
      "whilelt p9.b, x15, %x[width]\n"
      ".inst 0xe0b08b64  // st1w { za1v.s[x12] }, p2/Z, [x27, x16, LSL #2]\n"
      "incb x15\n"
      "add x26, x26, #0x10\n"
      "udot z19.s, z16.b, z20.b\n"
      ".inst 0xe0ae8761  // st1w { za0v.s[x12, #1] }, p1/Z, [x27, x14, LSL #2]\n"
      "add x25, x25, #0x10\n"
      "udot z18.s, z17.b, z20.b\n"
      "incb x28\n"
      ".inst 0xe0ab8365  // st1w { za1v.s[x12, #1] }, p0/Z, [x27, x11, LSL #2]\n"
      "addvl x27, x27, #4\n"
      "whilelt p8.b, x15, %x[width]\n"
      "mov x13, #0x0\n"
      "mov x12, #0x0\n"
      "cbz x9, 8f\n"
      "7:"  // K loop: Main loop: Second: Loop
      ".inst 0x25256140  // psel p0.b, p8.b/Z, p10.b[w13]\n"
      ".inst 0xe01c2300  // ld1b { za0h.b[x13] }, p0/Z, [x24, x28]\n"
      ".inst 0x252d6140  // psel p0.b, p8.b/Z, p10.b[w13, #1]\n"
      "ldr x24, [x26, #0x0]\n"
      ".inst 0xe01c22e1  // ld1b { za0h.b[x13, #1] }, p0/Z, [x23, x28]\n"
      ".inst 0x25656140  // psel p0.b, p8.b/Z, p10.b[w13, #4]\n"
      ".inst 0x256d6142  // psel p2.b, p8.b/Z, p10.b[w13, #5]\n"
      "ldr x23, [x25, #0x0]\n"
      ".inst 0xe01c22c4  // ld1b { za0h.b[x13, #4] }, p0/Z, [x22, x28]\n"
      ".inst 0x25306d21  // psel p1.s, p11.s/Z, p9.s[w12]\n"
      "ldr x22, [x26, #0x8]\n"
      ".inst 0x25306d20  // psel p0.s, p11.s/Z, p9.s[w12]\n"
      ".inst 0xe01c2aa5  // ld1b { za0h.b[x13, #5] }, p2/Z, [x21, x28]\n"
      "ldr x21, [x25, #0x8]\n"
      ".inst 0xe0bf8768  // st1w { za2v.s[x12] }, p1/Z, [x27, XZR, LSL #2]\n"
      ".inst 0xc0829110  // mova z16.s, p4/M, za2v.s[x12]\n"
      ".inst 0xc0829191  // mova z17.s, p4/M, za3v.s[x12]\n"
      "udot z19.s, z16.b, z20.b\n"
      ".inst 0xe0b0836c  // st1w { za3v.s[x12] }, p0/Z, [x27, x16, LSL #2]\n"
      ".inst 0x25706d20  // psel p0.s, p11.s/Z, p9.s[w12, #1]\n"
      "udot z18.s, z17.b, z20.b\n"
      ".inst 0xe0ae8369  // st1w { za2v.s[x12, #1] }, p0/Z, [x27, x14, LSL #2]\n"
      ".inst 0x25706d20  // psel p0.s, p11.s/Z, p9.s[w12, #1]\n"
      ".inst 0xc0829130  // mova z16.s, p4/M, za2v.s[x12, #1]\n"
      ".inst 0xe0ab836d  // st1w { za3v.s[x12, #1] }, p0/Z, [x27, x11, LSL #2]\n"
      ".inst 0xc08291b1  // mova z17.s, p4/M, za3v.s[x12, #1]\n"
      "add x12, x12, #0x2\n"
      "cmp x12, x9\n"
      "add x26, x26, #0x10\n"
      "add x25, x25, #0x10\n"
      "udot z19.s, z16.b, z20.b\n"
      "udot z18.s, z17.b, z20.b\n"
      "addvl x27, x27, #4\n"
      "add x13, x13, #0x8\n"
      "blt 7b\n"
      "8:"  // K loop: Main loop: Second: Tail
      ".inst 0x25256140  // psel p0.b, p8.b/Z, p10.b[w13]\n"
      ".inst 0xe01c2300  // ld1b { za0h.b[x13] }, p0/Z, [x24, x28]\n"
      ".inst 0x252d6140  // psel p0.b, p8.b/Z, p10.b[w13, #1]\n"
      ".inst 0xe01c22e1  // ld1b { za0h.b[x13, #1] }, p0/Z, [x23, x28]\n"
      ".inst 0x25656141  // psel p1.b, p8.b/Z, p10.b[w13, #4]\n"
      ".inst 0x256d6140  // psel p0.b, p8.b/Z, p10.b[w13, #5]\n"
      ".inst 0xe01c26c4  // ld1b { za0h.b[x13, #4] }, p1/Z, [x22, x28]\n"
      "mov x26, %x[in]\n"
      "add x25, %x[in], x16, LSL #3\n"
      "ldr x24, [x26, #0x0]\n"
      ".inst 0xe01c22a5  // ld1b { za0h.b[x13, #5] }, p0/Z, [x21, x28]\n"
      ".inst 0xc0829110  // mova z16.s, p4/M, za2v.s[x12]\n"
      ".inst 0x25306d23  // psel p3.s, p11.s/Z, p9.s[w12]\n"
      "udot z19.s, z16.b, z20.b\n"
      ".inst 0xc0829191  // mova z17.s, p4/M, za3v.s[x12]\n"
      "udot z18.s, z17.b, z20.b\n"
      "ldr x23, [x25, #0x0]\n"
      ".inst 0x25306d22  // psel p2.s, p11.s/Z, p9.s[w12]\n"
      "ldr x22, [x26, #0x8]\n"
      ".inst 0x25706d21  // psel p1.s, p11.s/Z, p9.s[w12, #1]\n"
      ".inst 0xc0829130  // mova z16.s, p4/M, za2v.s[x12, #1]\n"
      ".inst 0x25706d20  // psel p0.s, p11.s/Z, p9.s[w12, #1]\n"
      "ldr x21, [x25, #0x8]\n"
      ".inst 0xe0bf8f68  // st1w { za2v.s[x12] }, p3/Z, [x27, XZR, LSL #2]\n"
      ".inst 0xc08291b1  // mova z17.s, p4/M, za3v.s[x12, #1]\n"
      "whilelt p9.b, x15, %x[width]\n"
      ".inst 0xe0b08b6c  // st1w { za3v.s[x12] }, p2/Z, [x27, x16, LSL #2]\n"
      "subs x20, x20, #0x1\n"
      "add x26, x26, #0x10\n"
      "udot z19.s, z16.b, z20.b\n"
      ".inst 0xe0ae8769  // st1w { za2v.s[x12, #1] }, p1/Z, [x27, x14, LSL #2]\n"
      "add x25, x25, #0x10\n"
      "udot z18.s, z17.b, z20.b\n"
      "incb x15\n"
      ".inst 0xe0ab836d  // st1w { za3v.s[x12, #1] }, p0/Z, [x27, x11, LSL #2]\n"
      "addvl x27, x27, #4\n"
      "incb x28\n"
      "bgt 4b\n"
      "9:"  // K loop: Tails
      "cbnz x10, 12f\n"
      "mov x26, %x[in]\n"
      "whilelt p8.b, x15, %x[width]\n"
      "mov x13, #0x0\n"
      "mov x12, #0x0\n"
      "10:"  // K loop: Tails: Even: First
      ".inst 0x25306d20  // psel p0.s, p11.s/Z, p9.s[w12]\n"
      ".inst 0xe0bf8360  // st1w { za0v.s[x12] }, p0/Z, [x27, XZR, LSL #2]\n"
      ".inst 0x25306d20  // psel p0.s, p11.s/Z, p9.s[w12]\n"
      ".inst 0xc0829010  // mova z16.s, p4/M, za0v.s[x12]\n"
      ".inst 0xe0b08364  // st1w { za1v.s[x12] }, p0/Z, [x27, x16, LSL #2]\n"
      "ldr x21, [x26, #0x0]\n"
      ".inst 0x25356140  // psel p0.b, p8.b/Z, p10.b[w13, #2]\n"
      ".inst 0xc0829091  // mova z17.s, p4/M, za1v.s[x12]\n"
      "ldr x20, [x26, x16, LSL #0x3]\n"
      ".inst 0xe01c22a2  // ld1b { za0h.b[x13, #2] }, p0/Z, [x21, x28]\n"
      "add x12, x12, #0x1\n"
      ".inst 0x253d6140  // psel p0.b, p8.b/Z, p10.b[w13, #3]\n"
      "cmp x12, x16\n"
      "udot z19.s, z16.b, z20.b\n"
      "udot z18.s, z17.b, z20.b\n"
      ".inst 0xe01c2283  // ld1b { za0h.b[x13, #3] }, p0/Z, [x20, x28]\n"
      "add x26, x26, #0x8\n"
      "addvl x27, x27, #2\n"
      "add x13, x13, #0x4\n"
      "blt 10b\n"
      "whilelt p9.b, x15, %x[width]\n"
      "whilelt p8.b, x15, %x[width]\n"
      "mov x20, #0x0\n"
      "mov x12, #0x0\n"
      "11:"  // K loop: Tails: Even: Second
      ".inst 0x25306d20  // psel p0.s, p11.s/Z, p9.s[w12]\n"
      ".inst 0xe0bf8368  // st1w { za2v.s[x12] }, p0/Z, [x27, XZR, LSL #2]\n"
      ".inst 0x25306d20  // psel p0.s, p11.s/Z, p9.s[w12]\n"
      ".inst 0xc0829110  // mova z16.s, p4/M, za2v.s[x12]\n"
      ".inst 0xe0b0836c  // st1w { za3v.s[x12] }, p0/Z, [x27, x16, LSL #2]\n"
      ".inst 0xc0829191  // mova z17.s, p4/M, za3v.s[x12]\n"
      "add x12, x12, #0x1\n"
      "cmp x12, x17\n"
      "udot z19.s, z16.b, z20.b\n"
      "udot z18.s, z17.b, z20.b\n"
      "addvl x27, x27, #2\n"
      "add x20, x20, #0x4\n"
      "blt 11b\n"
      "whilelt p8.b, x15, %x[width]\n"
      "b 14f\n"
      "12:"  // K loop: Tails: Odd
      "mov x12, #0x0\n"
      "13:"  // K loop: Tails: Odd: Loop
      ".inst 0x25306d20  // psel p0.s, p11.s/Z, p9.s[w12]\n"
      ".inst 0xe0bf8360  // st1w { za0v.s[x12] }, p0/Z, [x27, XZR, LSL #2]\n"
      ".inst 0x25306d20  // psel p0.s, p11.s/Z, p9.s[w12]\n"
      ".inst 0xc0829010  // mova z16.s, p4/M, za0v.s[x12]\n"
      ".inst 0xe0b08364  // st1w { za1v.s[x12] }, p0/Z, [x27, x16, LSL #2]\n"
      ".inst 0xc0829091  // mova z17.s, p4/M, za1v.s[x12]\n"
      "add x12, x12, #0x1\n"
      "cmp x12, x17\n"
      "udot z19.s, z16.b, z20.b\n"
      "udot z18.s, z17.b, z20.b\n"
      "addvl x27, x27, #2\n"
      "blt 13b\n"
      "14:"  // K loop: End
      "st1w { z19.s }, p4, [x27]\n"
      "st1w { z18.s }, p4, [x27, #1, MUL VL]\n"
      "addvl x27, x27, #2\n"
      "mov %x[out], x27\n"
      ".inst 0xd503467f  // SMSTOP\n"
      : [out] "+&r" (out)
      : [first] "r" (first), [height] "r" (height), [in] "r" (in), [row_offset] "r" (row_offset), [width] "r" (width)
      : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
    );
}

#endif  // defined(ARM_COMPUTE_ENABLE_SME)
