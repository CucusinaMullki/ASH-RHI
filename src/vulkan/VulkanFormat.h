#pragma once

#include "ASH/Types.h"
#include <vulkan/vulkan.h>

namespace ASH::vulkan {

struct BarrierMasks
{
    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;
    VkAccessFlags srcAccess;
    VkAccessFlags dstAccess;
};

BarrierMasks toBarrierMasks(ASH::ResourceState oldState, ASH::ResourceState newState);
VkFormat toVkFormat(ASH::Format format);
VkBufferUsageFlags toVkBufferUsage(ASH::BufferUsage usage);
VkImageUsageFlags toVkImageUsage(ASH::TextureUsage usage);
VkImageType toVkImageType(ASH::TextureType type);
VkImageViewType toVkImageViewType(ASH::TextureType type);
VkShaderStageFlagBits toVkShaderStage(ASH::ShaderStage stage);
VkPrimitiveTopology toVkPrimitiveTopology(ASH::PrimitiveTopology topology);
VkPolygonMode toVkPolygonMode(ASH::PolygonMode mode);
VkCullModeFlags toVkCullMode(ASH::CullMode mode);
VkFrontFace toVkFrontFace(ASH::FrontFace face);
VkCompareOp toVkCompareOp(ASH::CompareOp op);
VkBlendFactor toVkBlendFactor(ASH::BlendFactor factor);
VkBlendOp toVkBlendOp(ASH::BlendOp op);
VkAttachmentLoadOp toVkLoadOp(ASH::LoadOp op);
VkAttachmentStoreOp toVkStoreOp(ASH::StoreOp op);
VkIndexType toVkIndexType(ASH::IndexType type);
VkFilter toVkFilter(ASH::Filter filter);
VkSamplerAddressMode toVkAddressMode(ASH::AddressMode mode);
VkPresentModeKHR toVkPresentMode(ASH::PresentMode mode);
VkImageLayout toVkImageLayout(ASH::ResourceState state);

bool isDepthFormat(ASH::Format format);
bool isStencilFormat(ASH::Format format);

}