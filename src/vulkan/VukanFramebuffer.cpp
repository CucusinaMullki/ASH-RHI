#include "VulkanFramebuffer.h"
#include "VulkanTexture.h"
#include "VulkanRenderPass.h"
#include "VulkanResult.h"
#include <vector>

namespace ASH::vulkan
{

VulkanFramebuffer::VulkanFramebuffer(VkDevice device, const ASH::FramebufferDesc& desc)
    : m_device(device)
    , m_extent(desc.extent)
{
    std::vector<VkImageView> attachments;
    attachments.reserve(desc.colorAttachment.size() + 1);

    for (ASH::Texture* colorTexture : desc.colorAttachment)
    {
        auto* vulkanTexture = static_cast<VulkanTexture*>(colorTexture);
        attachments.push_back(vulkanTexture->getImageView());
    }

    if (desc.depthStencilAttachment != nullptr)
    {
        auto* vulkanTexture = static_cast<VulkanTexture*>(desc.depthStencilAttachment);
        attachments.push_back(vulkanTexture->getImageView());
    }

    auto* vulkanRenderPass = static_cast<VulkanRenderPass*>(desc.renderPass);

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vulkanRenderPass->getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = desc.extent.width;
    framebufferInfo.height = desc.extent.height;
    framebufferInfo.layers = desc.layers;

    VK_CHECK(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffer), "vkCreateFramebuffer");
}

VulkanFramebuffer::~VulkanFramebuffer()
{
    if (m_framebuffer != VK_NULL_HANDLE) vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
}

}