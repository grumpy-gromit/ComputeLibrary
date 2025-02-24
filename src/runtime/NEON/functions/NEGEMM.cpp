/*
 * Copyright (c) 2017-2024 Arm Limited.
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
#include "arm_compute/runtime/NEON/functions/NEGEMM.h"

#include "arm_compute/core/ITensorPack.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/runtime/MemoryGroup.h"
#include "arm_compute/runtime/Tensor.h"

#include "src/core/CPP/Validate.h"
#include "src/core/helpers/MemoryHelpers.h"
#include "src/cpu/operators/CpuGemm.h"

using namespace arm_compute::experimental;

namespace arm_compute
{
struct NEGEMM::Impl
{
    MemoryGroup      memory_group{};
    IWeightsManager *weights_manager{nullptr};

    std::unique_ptr<cpu::CpuGemm> op{nullptr};

    const ITensor *original_b{nullptr};
    bool           is_prepared{false};

    ITensorPack                      run_pack{};
    ITensorPack                      prep_pack{};
    WorkspaceData<Tensor>            workspace{};
    experimental::MemoryRequirements aux_mem_req{};
};

NEGEMM::NEGEMM(std::shared_ptr<IMemoryManager> memory_manager, IWeightsManager *weights_manager)
    : _impl(std::make_unique<Impl>())
{
    _impl->memory_group    = MemoryGroup(std::move(memory_manager));
    _impl->weights_manager = weights_manager;
}

NEGEMM::~NEGEMM() = default;

void NEGEMM::configure(const ITensor  *a,
                       const ITensor  *b,
                       const ITensor  *c,
                       ITensor        *d,
                       float           alpha,
                       float           beta,
                       const GEMMInfo &gemm_info)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(a, b, d);
    ARM_COMPUTE_ERROR_THROW_ON(cpu::CpuGemm::validate(a->info(), b->info(), (c != nullptr) ? c->info() : nullptr,
                                                      d->info(), alpha, beta, gemm_info));

    // Check if we need to reshape the matrix B only on the first run
    _impl->is_prepared = false;
    _impl->memory_group.mappings().clear();
    _impl->original_b = b;
    _impl->op         = std::make_unique<cpu::CpuGemm>();

    // Make the B matrix dynamic values.
    auto b_info_to_use = b->info()->clone();
    if (!gemm_info.reshape_b_only_on_first_run())
    {
        b_info_to_use->set_are_values_constant(false);
    }

    _impl->op->configure(a->info(), b_info_to_use.get(), (c != nullptr) ? c->info() : nullptr, d->info(), alpha, beta,
                         gemm_info);

    _impl->aux_mem_req = _impl->op->workspace();
    _impl->run_pack    = {{ACL_SRC_0, a}, {ACL_SRC_1, b}, {ACL_SRC_2, c}, {ACL_DST, d}};
    _impl->prep_pack   = {{ACL_SRC_1, b}, {ACL_SRC_2, c}};
    _impl->workspace   = manage_workspace<Tensor>(_impl->aux_mem_req, _impl->memory_group, _impl->run_pack,
                                                _impl->prep_pack, /* allocate_now */ false);
}

Status NEGEMM::validate(const ITensorInfo *a,
                        const ITensorInfo *b,
                        const ITensorInfo *c,
                        const ITensorInfo *output,
                        float              alpha,
                        float              beta,
                        const GEMMInfo    &gemm_info)
{
    ARM_COMPUTE_RETURN_ERROR_ON_DYNAMIC_SHAPE(a, b, c, output);
    // Make the B matrix dynamic values.
    auto b_to_use = b->clone();
    if (!gemm_info.reshape_b_only_on_first_run())
    {
        b_to_use->set_are_values_constant(false);
    }

    return cpu::CpuGemm::validate(a, b_to_use.get(), c, output, alpha, beta, gemm_info);
}

Status NEGEMM::has_opt_impl(arm_compute::WeightFormat &expected_weight_format,
                            const ITensorInfo         *a,
                            const ITensorInfo         *b,
                            const ITensorInfo         *c,
                            const ITensorInfo         *output,
                            float                      alpha,
                            float                      beta,
                            const GEMMInfo            &gemm_info)
{
    ARM_COMPUTE_UNUSED(alpha, beta);
    return cpu::CpuGemm::has_opt_impl(expected_weight_format, a, b, c, output, gemm_info);
}

void NEGEMM::run()
{
    prepare();

    MemoryGroupResourceScope scope_mg(_impl->memory_group);
    _impl->op->run(_impl->run_pack);
}

void NEGEMM::prepare()
{
    if (!_impl->is_prepared)
    {
        allocate_tensors(_impl->aux_mem_req, _impl->workspace);
        _impl->op->prepare(_impl->prep_pack);

        auto has_reshape =
            std::find_if(_impl->aux_mem_req.begin(), _impl->aux_mem_req.end(),
                         [](const MemoryInfo &m) -> bool { return m.lifetime == MemoryLifetime::Persistent; });

        if (has_reshape != std::end(_impl->aux_mem_req))
        {
            _impl->original_b->mark_as_unused();
        }
        else
        {
            _impl->run_pack.add_const_tensor(ACL_SRC_1, _impl->original_b);
        }

        // Release temporary tensors that are only used in prepare stage
        release_temporaries<Tensor>(_impl->aux_mem_req, _impl->workspace);
        _impl->is_prepared = true;
    }
}
} // namespace arm_compute
