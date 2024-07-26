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
#pragma once
#ifdef ARM_COMPUTE_ENABLE_SVE

#include "../std_transforms_sve.hpp"
#include "../bfloat.hpp"
#include "../performance_parameters.hpp"

#define ARGLIST  \
    const bfloat16 *, const bfloat16 *, \
    float *, int, int, int

namespace arm_gemm
{
// Actual kernel implementations
void sve_interleaved_bf16fp32_dot_8x3VL( ARGLIST );

class cls_sve_interleaved_bf16fp32_dot_8x3VL
{
public:
    typedef bfloat16 operand_type;
    typedef float result_type;

    typedef void (*kern_type)( ARGLIST );

    /* Kernel blocking parameters */
    static constexpr unsigned int out_height()
    {
        return 8;
    }

    static unsigned int out_width()
    {
        return get_vector_length<float>() * 3;
    }

    static constexpr unsigned int k_unroll()
    {
        return 2;
    }


    StdTransformsSVE<operand_type, result_type, 8, 3, 2, 1> transforms = {};
    StdTransformsSVE<operand_type, result_type, 8, 3, 2, 1, true> transforms_quantized = {};
    template<typename T>
    static inline PerformanceParameters get_performance_parameters(const CPUInfo *ci)
    {

        if (std::is_same<T, bfloat16>::value) {
            switch (ci->get_cpu_model()) {
                default:
                    return { 15.92, 3.74, 7.14 };
                case CPUModel::A510:
                    return { 7.54, 3.77, 2.43 };
                case CPUModel::V1:
                    return { 31.82, 5.11, 11.20 };
            }
        }

        return { 1.0 };
    }

    // Default to the generic kernel
    kern_type kernel=sve_interleaved_bf16fp32_dot_8x3VL;
    cls_sve_interleaved_bf16fp32_dot_8x3VL(const CPUInfo *)
    {
    }
};

} // namespace arm_gemm

#undef ARGLIST
#endif // ARM_COMPUTE_ENABLE_SVE
