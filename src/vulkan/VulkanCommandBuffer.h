#pragma once

#include "ASH/CommandBuffer.h"
#include <vulkan/vulkan.h>


namespace ASH::vulkan {

class VulkanCommandBuffer final : public ASH::CommandBuffer
{
public:
    VulkanCommandBuffer(VkDevice device, VkCommandPool commandPool);
    ~VulkanCommandBuffer() override;

    void begin() override;
    void end()   override;

    void beginRenderPass(const ASH::RenderPassBeginInfo& info) override;
    void endRenderPass() override;

    void bindPipeline(ASH::Pipeline* pipeline) override;
    void bindDescriptorSet(ASH::Pipeline* pipeline, uint32_t setIndex, ASH::DescriptorSet* set) override;
    void pushConstants(ASH::Pipeline* pipeline, ASH::ShaderStage stages, uint32_t offset, uint32_t size, const void* data) override;
    void bindVertexBuffer(ASH::Buffer* buffer, uint32_t binding, uint64_t offset) override;
    void bindIndexBuffer(ASH::Buffer* buffer, uint64_t offset, ASH::IndexType indexType) override;

    void setViewport(const ASH::Viewport& viewport) override;
    void setScissor(const ASH::Rect2D& scissor) override;

    void draw(uint32_t vertexCount, uint32_t instanceCount,
        uint32_t firstVertex, uint32_t firstInstance) override;

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount,
        uint32_t firstIndex, int32_t vertexOffset,
        uint32_t firstInstance) override;

    void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

    void copyBuffer(ASH::Buffer* src, ASH::Buffer* dst, size_t size,
        size_t srcOffset, size_t dstOffset) override;

    void copyBufferToTexture(ASH::Buffer* src, ASH::Texture* dst) override;

    void barrier(const ASH::TextureBarrier* textureBarriers, uint32_t textureBarrierCount,
        const ASH::BufferBarrier*  bufferBarriers,  uint32_t bufferBarrierCount) override;

    VkCommandBuffer getHandle() const { return m_commandBuffer; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
};

}