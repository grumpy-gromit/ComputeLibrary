/*
 * Copyright (c) 2017-2021, 2024 Arm Limited.
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

#include "arm_compute/runtime/NEON/functions/NEBatchNormalizationLayer.h"

#include "arm_compute/core/Error.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/core/Validate.h"
#include "arm_compute/runtime/NEON/NEScheduler.h"

#include "src/common/utils/Log.h"
#include "src/core/NEON/kernels/NEBatchNormalizationLayerKernel.h"

namespace arm_compute
{
NEBatchNormalizationLayer::~NEBatchNormalizationLayer() = default;

NEBatchNormalizationLayer::NEBatchNormalizationLayer() : _norm_kernel()
{
}

void NEBatchNormalizationLayer::configure(ITensor            *input,
                                          ITensor            *output,
                                          const ITensor      *mean,
                                          const ITensor      *var,
                                          const ITensor      *beta,
                                          const ITensor      *gamma,
                                          float               epsilon,
                                          ActivationLayerInfo act_info)
{
    ARM_COMPUTE_LOG_PARAMS(input, output, mean, var, beta, gamma, epsilon, act_info);
    // Configure kernel
    _norm_kernel = std::make_unique<NEBatchNormalizationLayerKernel>();
    _norm_kernel->configure(input, output, mean, var, beta, gamma, epsilon, act_info);
}

Status NEBatchNormalizationLayer::validate(const ITensorInfo  *input,
                                           const ITensorInfo  *output,
                                           const ITensorInfo  *mean,
                                           const ITensorInfo  *var,
                                           const ITensorInfo  *beta,
                                           const ITensorInfo  *gamma,
                                           float               epsilon,
                                           ActivationLayerInfo act_info)
{
    ARM_COMPUTE_RETURN_ERROR_ON_DYNAMIC_SHAPE(input, output, mean, var, beta, gamma);
    ARM_COMPUTE_RETURN_ON_ERROR(
        NEBatchNormalizationLayerKernel::validate(input, output, mean, var, beta, gamma, epsilon, act_info));
    return Status{};
}

void NEBatchNormalizationLayer::run()
{
    NEScheduler::get().schedule(_norm_kernel.get(), Window::DimY);
}
} // namespace arm_compute
