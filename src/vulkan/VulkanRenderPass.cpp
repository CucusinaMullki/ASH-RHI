#include "VulkanRenderPass.h"
#include "VulkanFormat.h"
#include "VulkanResult.h"
#include <vector>

namespace ASH::vulkan
{

namespace
{

VkAttachmentDescription toAttachmentDescription(const ASH::AttachmentDesc& desc, VkImageLayout finalLayout)
{
    VkAttachmentDescription attachment{};
    attachment.format = toVkFormat(desc.format);
    attachment.samples = static_cast<VkSampleCountFlagBits>(desc.sampleCount);
    attachment.loadOp = toVkLoadOp(desc.loadOp);
    attachment.storeOp = toVkStoreOp(desc.storeOp);
    attachment.stencilLoadOp = toVkLoadOp(desc.stencilLoadOp);
    attachment.stencilStoreOp = toVkStoreOp(desc.stencilStoreOp);

    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    attachment.finalLayout = finalLayout;

    return attachment;
}

}

VulkanRenderPass::VulkanRenderPass(VkDevice device, const ASH::RenderPassDesc& desc)
    : m_device(device)
    , m_desc(desc)
{
    std::vector<VkAttachmentDescription> attachments;

    std::vector<VkAttachmentReference> colorRefs;

    for (size_t i = 0; i < desc.colorAttachments.size(); ++i)
    {
        attachments.push_back(toAttachmentDescription(desc.colorAttachments[i],
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));

        VkAttachmentReference ref{};
        ref.attachment = static_cast<uint32_t>(i);
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorRefs.push_back(ref);
    }

    VkAttachmentReference depthRef{};
    if (desc.hasDepthStencil) {
        attachments.push_back(toAttachmentDescription(desc.depthStencilAttachment,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
        depthRef.attachment = static_cast<uint32_t>(attachments.size() - 1);
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
    subpass.pColorAttachments = colorRefs.data();
    subpass.pDepthStencilAttachment = desc.hasDepthStencil ? &depthRef : nullptr;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass), "vkCreateRenderPass");
}

VulkanRenderPass::~VulkanRenderPass()
{
    if (m_renderPass != VK_NULL_HANDLE) vkDestroyRenderPass(m_device, m_renderPass, nullptr);
}

}