#pragma once

#include "ASH/Types.h"
#include <cstddef>
#include <cstdint>

namespace ASH
{

class Pipeline;
class Buffer;
class Texture;
class RenderPass;
class DescriptorSet;
class Framebuffer;

struct RenderPassBeginInfo
{
    RenderPass* renderPass = nullptr;
    Framebuffer* framebuffer = nullptr;
    Rect2D renderArea {};

    const ClearColor* colorClearValues = nullptr;
    uint32_t colorClearCount = 0;
    const ClearDepthStencil* depthStencilClear = nullptr;
};

struct TextureBarrier
{
    Texture* texture = nullptr;
    ResourceState oldState = ResourceState::Undefined;
    ResourceState newState = ResourceState::Undefined;
};

struct BufferBarrier
{
    Buffer* buffer = nullptr;
};

class CommandBuffer
{
public:
    virtual ~CommandBuffer() = default;

    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    virtual void begin() = 0;
    virtual void end() = 0;

    virtual void beginRenderPass(const RenderPassBeginInfo& info) = 0;
    virtual void endRenderPass() = 0;

    virtual void bindPipeline(Pipeline* pipeline) = 0;
    virtual void bindDescriptorSet(Pipeline* pipeline, uint32_t setIndex, DescriptorSet* set) = 0;
    virtual void bindVertexBuffer(Buffer* buffer, uint32_t binding, uint64_t offset) = 0;
    virtual void bindIndexBuffer(Buffer* buffer, uint64_t offset, IndexType indexType) = 0;

    virtual void setViewport(const Viewport& viewport) = 0;
    virtual void setScissor(const Rect2D& scissor) = 0;

    virtual void draw(uint32_t vetexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstnace) = 0;

    virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;

    virtual void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

    virtual void copyBuffer(Buffer* src, Buffer* dst, size_t size, size_t srcOffset, size_t dstOffset) = 0;

    virtual void copyBufferToTexture(Buffer* src, Texture* dst) = 0;

    virtual void pushConstants(Pipeline* pipeline, ShaderStage stages, uint32_t offset, uint32_t size, const void* data) = 0;

    virtual void barrier(const TextureBarrier* textureBarriers, uint32_t textureBarrierCount, const BufferBarrier* bufferBarriers, uint32_t bufferBarrierCount) = 0;

protected:
    CommandBuffer() = default;
};

}