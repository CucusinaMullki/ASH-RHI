#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptorSet.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanFormat.h"
#include "VulkanResult.h"
#include <vector>

namespace ASH::vulkan
{

namespace
{

VkPipelineBindPoint toBindPoint(ASH::PipelineType type)
{
    switch (type)
    {
        case ASH::PipelineType::Graphics: return VK_PIPELINE_BIND_POINT_GRAPHICS;
        case ASH::PipelineType::Compute: return VK_PIPELINE_BIND_POINT_COMPUTE;
    }
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
}

VkImageAspectFlags aspectMaskFor(const ASH::TextureDesc& desc)
{
    if (!isDepthFormat(desc.format))
    {
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageAspectFlags mask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (isStencilFormat(desc.format))
    {
        mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    return mask;
}

}

void VulkanCommandBuffer::pushConstants(ASH::Pipeline* pipeline, ASH::ShaderStage stages, uint32_t offset, uint32_t size, const void* data)
{
    auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);

    VkShaderStageFlags vkStages = 0;
    if (hasFlag(stages, ASH::ShaderStage::Vertex)) vkStages |= VK_SHADER_STAGE_VERTEX_BIT;
    if (hasFlag(stages, ASH::ShaderStage::Fragment)) vkStages |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (hasFlag(stages, ASH::ShaderStage::Compute)) vkStages |= VK_SHADER_STAGE_COMPUTE_BIT;
    if (hasFlag(stages, ASH::ShaderStage::Geometry)) vkStages |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if (hasFlag(stages, ASH::ShaderStage::TessControl)) vkStages |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if (hasFlag(stages, ASH::ShaderStage::TessEval)) vkStages |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    vkCmdPushConstants(m_commandBuffer, vulkanPipeline->getLayout(), vkStages, offset, size, data);
}

VulkanCommandBuffer::VulkanCommandBuffer(VkDevice device, VkCommandPool commandPool)
    : m_device(device)
    , m_commandPool(commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    allocInfo.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer), "vkAllocateCommandBuffers");
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    if (m_commandBuffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandBuffer);
    }
}

void VulkanCommandBuffer::begin()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo), "vkBeginCommandBuffer");
}

void VulkanCommandBuffer::end()
{
    VK_CHECK(vkEndCommandBuffer(m_commandBuffer), "vkEndCommandBuffer");
}

void VulkanCommandBuffer::beginRenderPass(const ASH::RenderPassBeginInfo& info)
{
    auto* renderPass  = static_cast<VulkanRenderPass*>(info.renderPass);
    auto* framebuffer = static_cast<VulkanFramebuffer*>(info.framebuffer);

    std::vector<VkClearValue> clearValues;
    clearValues.reserve(info.colorClearCount + 1);

    for (uint32_t i = 0; i < info.colorClearCount; ++i)
    {
        VkClearValue clear{};
        clear.color.float32[0] = info.colorClearValues[i].f32[0];
        clear.color.float32[1] = info.colorClearValues[i].f32[1];
        clear.color.float32[2] = info.colorClearValues[i].f32[2];
        clear.color.float32[3] = info.colorClearValues[i].f32[3];
        clearValues.push_back(clear);
    }

    if (info.depthStencilClear != nullptr)
    {
        VkClearValue clear{};
        clear.depthStencil.depth = info.depthStencilClear->depth;
        clear.depthStencil.stencil = info.depthStencilClear->stencil;
        clearValues.push_back(clear);
    }

    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = renderPass->getHandle();
    beginInfo.framebuffer = framebuffer->getHandle();
    beginInfo.renderArea.offset = { info.renderArea.x, info.renderArea.y };
    beginInfo.renderArea.extent = { info.renderArea.width, info.renderArea.height };
    beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    beginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::endRenderPass()
{
    vkCmdEndRenderPass(m_commandBuffer);
}

void VulkanCommandBuffer::bindPipeline(ASH::Pipeline* pipeline)
{
    auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
    vkCmdBindPipeline(m_commandBuffer, toBindPoint(pipeline->getType()), vulkanPipeline->getHandle());
}

void VulkanCommandBuffer::bindVertexBuffer(ASH::Buffer* buffer, uint32_t binding, uint64_t offset)
{
    auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
    VkBuffer     handle   = vulkanBuffer->getHandle();
    VkDeviceSize vkOffset = offset;

    vkCmdBindVertexBuffers(m_commandBuffer, binding, 1, &handle, &vkOffset);
}

void VulkanCommandBuffer::bindIndexBuffer(ASH::Buffer* buffer, uint64_t offset, ASH::IndexType indexType)
{
    auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
    vkCmdBindIndexBuffer(m_commandBuffer, vulkanBuffer->getHandle(), offset, toVkIndexType(indexType));
}

void VulkanCommandBuffer::setViewport(const ASH::Viewport& viewport)
{
    VkViewport vkViewport{};
    vkViewport.x = viewport.x;
    vkViewport.y = viewport.y;
    vkViewport.width = viewport.width;
    vkViewport.height = viewport.height;
    vkViewport.minDepth = viewport.minDepth;
    vkViewport.maxDepth = viewport.maxDepth;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
}

void VulkanCommandBuffer::setScissor(const ASH::Rect2D& scissor)
{
    VkRect2D vkScissor{};
    vkScissor.offset = { scissor.x, scissor.y };
    vkScissor.extent = { scissor.width, scissor.height };
    vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
}

void VulkanCommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount,
    uint32_t firstVertex, uint32_t firstInstance)
{
    vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount,
    uint32_t firstIndex, int32_t vertexOffset,
    uint32_t firstInstance)
{
    vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    vkCmdDispatch(m_commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void VulkanCommandBuffer::copyBuffer(ASH::Buffer* src, ASH::Buffer* dst, size_t size,
    size_t srcOffset, size_t dstOffset)
{
    auto* vulkanSrc = static_cast<VulkanBuffer*>(src);
    auto* vulkanDst = static_cast<VulkanBuffer*>(dst);

    VkBufferCopy region{};
    region.srcOffset = srcOffset;
    region.dstOffset = dstOffset;
    region.size = size;

    vkCmdCopyBuffer(m_commandBuffer, vulkanSrc->getHandle(), vulkanDst->getHandle(), 1, &region);
}

void VulkanCommandBuffer::bindDescriptorSet(ASH::Pipeline* pipeline, uint32_t setIndex, ASH::DescriptorSet* set) {
    auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
    auto* vulkanSet       = static_cast<VulkanDescriptorSet*>(set);

    VkDescriptorSet handle = vulkanSet->getHandle();

    vkCmdBindDescriptorSets(
        m_commandBuffer,
        toBindPoint(pipeline->getType()),
        vulkanPipeline->getLayout(),
        setIndex,
        1,
        &handle,
        0, nullptr
    );
}

void VulkanCommandBuffer::copyBufferToTexture(ASH::Buffer* src, ASH::Texture* dst)
{
    auto* vulkanSrc = static_cast<VulkanBuffer*>(src);
    auto* vulkanDst = static_cast<VulkanTexture*>(dst);
    const ASH::TextureDesc& desc = vulkanDst->getDesc();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.imageSubresource.aspectMask = aspectMaskFor(desc);
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = desc.arrayLayers;
    region.imageExtent = { desc.extent.width, desc.extent.height, desc.extent.depth };

    vkCmdCopyBufferToImage(m_commandBuffer, vulkanSrc->getHandle(), vulkanDst->getImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanCommandBuffer::barrier(const ASH::TextureBarrier* textureBarriers, uint32_t textureBarrierCount,
    const ASH::BufferBarrier*  bufferBarriers,  uint32_t bufferBarrierCount)
{
    std::vector<VkImageMemoryBarrier> imageBarriers;
    imageBarriers.reserve(textureBarrierCount);

    VkPipelineStageFlags combinedSrcStage = 0;
    VkPipelineStageFlags combinedDstStage = 0;

    for (uint32_t i = 0; i < textureBarrierCount; ++i)
    {
        const ASH::TextureBarrier& b = textureBarriers[i];
        auto* vulkanTexture = static_cast<VulkanTexture*>(b.texture);
        const ASH::TextureDesc& desc = vulkanTexture->getDesc();

        BarrierMasks masks = toBarrierMasks(b.oldState, b.newState);
        combinedSrcStage |= masks.srcStage;
        combinedDstStage |= masks.dstStage;

        VkImageMemoryBarrier imageBarrier{};
        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrier.oldLayout = toVkImageLayout(b.oldState);
        imageBarrier.newLayout = toVkImageLayout(b.newState);
        imageBarrier.srcAccessMask = masks.srcAccess;
        imageBarrier.dstAccessMask = masks.dstAccess;
        imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.image = vulkanTexture->getImage();
        imageBarrier.subresourceRange.aspectMask = aspectMaskFor(desc);
        imageBarrier.subresourceRange.baseMipLevel = 0;
        imageBarrier.subresourceRange.levelCount = desc.mipLevels;
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount = desc.arrayLayers;

        imageBarriers.push_back(imageBarrier);
    }

    (void)bufferBarriers;
    (void)bufferBarrierCount;

    if (combinedSrcStage == 0) combinedSrcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    if (combinedDstStage == 0) combinedDstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkCmdPipelineBarrier(
        m_commandBuffer,
        combinedSrcStage,
        combinedDstStage,
        0,
        0, nullptr,
        0, nullptr,
        static_cast<uint32_t>(imageBarriers.size()), imageBarriers.data()
    );
}
}