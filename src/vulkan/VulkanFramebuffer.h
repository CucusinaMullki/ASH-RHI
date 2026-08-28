#pragma once

#include "ASH/Framebuffer.h"
#include <vulkan/vulkan.h>

namespace ASH::vulkan
{

class VulkanFramebuffer final : public ASH::Framebuffer
{
public:
    VulkanFramebuffer(VkDevice device, const ASH::FramebufferDesc& desc);
    ~VulkanFramebuffer() override;

    ASH::Extent2D getExtent() const override { return m_extent; }

    VkFramebuffer getHandle() const { return m_framebuffer; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
    ASH::Extent2D m_extent;
};

}