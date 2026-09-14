#include "VulkanTexture.h"
#include "VulkanFormat.h"
#include "VulkanResult.h"

namespace ASH::vulkan {

VulkanTexture::VulkanTexture(VkDevice device, VulkanMemoryAllocator* allocator, const ASH::TextureDesc& desc)
    : m_device(device)
    , m_allocator(allocator)
    , m_desc(desc)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = toVkImageType(desc.type);
    imageInfo.extent = { desc.extent.width, desc.extent.height, desc.extent.depth };
    imageInfo.mipLevels = desc.mipLevels;
    imageInfo.arrayLayers = desc.arrayLayers;
    imageInfo.format = toVkFormat(desc.format);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = toVkImageUsage(desc.usage);
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = static_cast<VkSampleCountFlagBits>(desc.sampleCount);

    if (desc.type == ASH::TextureType::TextureCube)
    {
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    VK_CHECK(vkCreateImage(m_device, &imageInfo, nullptr, &m_image), "vkCreateImage");

    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(m_device, m_image, &memRequirements);

    m_allocation = m_allocator->allocate(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkBindImageMemory(m_device, m_image, m_allocation.memory, m_allocation.offset), "vkBindImageMemory");

    createImageView();
}

VulkanTexture::VulkanTexture(VkDevice device, VkImage externalImage, const ASH::TextureDesc& desc)
    : m_device(device)
    , m_allocator(nullptr)
    , m_image(externalImage)
    , m_desc(desc)
    , m_ownsImage(false)
{
    createImageView();
}

VulkanTexture::~VulkanTexture()
{
    for (auto& [key, view] : m_faceViews)
        vkDestroyImageView(m_device, view, nullptr);

    if (m_imageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_imageView, nullptr);
    if (m_ownsImage && m_image != VK_NULL_HANDLE) vkDestroyImage(m_device, m_image, nullptr);

    if (m_allocator != nullptr)
    {
        m_allocator->free(m_allocation);
    }
}

void* VulkanTexture::getFaceView(uint32_t layer, uint32_t mipLevel)
{
    auto key = std::make_pair(layer, mipLevel);
    auto it = m_faceViews.find(key);
    if (it != m_faceViews.end())
        return it->second;

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    if (isDepthFormat(m_desc.format))
    {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (isStencilFormat(m_desc.format))
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = (m_desc.type == ASH::TextureType::Texture3D) ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = toVkFormat(m_desc.format);
    viewInfo.subresourceRange.aspectMask = aspectMask;
    viewInfo.subresourceRange.baseMipLevel = mipLevel;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = layer;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView view = VK_NULL_HANDLE;
    VK_CHECK(vkCreateImageView(m_device, &viewInfo, nullptr, &view), "vkCreateImageView (face view)");

    m_faceViews[key] = view;
    return view;
}

void VulkanTexture::createImageView()
{
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    if (isDepthFormat(m_desc.format))
    {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (isStencilFormat(m_desc.format))
        {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = toVkImageViewType(m_desc.type);
    viewInfo.format = toVkFormat(m_desc.format);
    viewInfo.subresourceRange.aspectMask = aspectMask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = m_desc.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = m_desc.arrayLayers;

    VK_CHECK(vkCreateImageView(m_device, &viewInfo, nullptr, &m_imageView), "vkCreateImageView");
}

}