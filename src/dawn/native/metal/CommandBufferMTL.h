// Copyright 2017 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_DAWN_NATIVE_METAL_COMMANDBUFFERMTL_H_
#define SRC_DAWN_NATIVE_METAL_COMMANDBUFFERMTL_H_

#include <set>
#include <utility>

#include "dawn/native/CommandBuffer.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Error.h"

#import <Metal/Metal.h>

namespace dawn::native {
class CommandEncoder;
struct BeginComputePassCmd;
struct BeginRenderPassCmd;
}  // namespace dawn::native

namespace dawn::native::metal {

class CommandRecordingContext;
class Device;
class Texture;
class QuerySet;

void RecordCopyBufferToTexture(CommandRecordingContext* commandContext,
                               id<MTLBuffer> mtlBuffer,
                               uint64_t bufferSize,
                               uint64_t offset,
                               uint32_t bytesPerRow,
                               uint32_t rowsPerImage,
                               Texture* texture,
                               uint32_t mipLevel,
                               const Origin3D& origin,
                               Aspect aspect,
                               const Extent3D& copySize);

class CommandBuffer final : public CommandBufferBase {
  public:
    static Ref<CommandBuffer> Create(CommandEncoder* encoder,
                                     const CommandBufferDescriptor* descriptor);

    CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor);
    ~CommandBuffer() override;

    MaybeError FillCommands(CommandRecordingContext* commandContext);

  private:
    using CommandBufferBase::CommandBufferBase;

    MaybeError EncodeComputePass(CommandRecordingContext* commandContext,
                                 BeginComputePassCmd* computePassCmd);

    // Empty occlusion queries aren't filled to zero on Apple GPUs. This set is used to
    // track which results should be explicitly zero'ed as a workaround. Use of empty queries
    // *should* mostly be a degenerate scenario, so this std::set shouldn't be performance-critical.
    // The set is passed as nullptr to `EncodeRenderPass` if the workaround is not in use.
    using EmptyOcclusionQueries = std::set<std::pair<QuerySet*, uint32_t>>;
    MaybeError EncodeRenderPass(id<MTLRenderCommandEncoder> encoder,
                                BeginRenderPassCmd* renderPassCmd,
                                EmptyOcclusionQueries* emptyOcclusionQueries);
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_COMMANDBUFFERMTL_H_
