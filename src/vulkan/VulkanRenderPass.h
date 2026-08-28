#pragma once

#include "ASH/RenderPass.h"
#include <vulkan/vulkan.h>

namespace ASH::vulkan
{

class VulkanRenderPass final : public ASH::RenderPass
{
public:
    VulkanRenderPass(VkDevice device, const ASH::RenderPassDesc& desc);
    ~VulkanRenderPass() override;
    const ASH::RenderPassDesc& getDesc() const override { return m_desc; }
    VkRenderPass getHandle() const { return m_renderPass; }
private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    ASH::RenderPassDesc m_desc;
};

}