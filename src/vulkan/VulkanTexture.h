#pragma once

#include "ASH/Texture.h"
#include <vulkan/vulkan.h>

namespace ASH::vulkan
{

class VulkanTexture final : public ASH::Texture
{
public:
    VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice, const ASH::TextureDesc& desc);

    VulkanTexture(VkDevice device, VkImage externalImage, const ASH::TextureDesc& desc);

    ~VulkanTexture() override;

    const ASH::TextureDesc& getDesc() const override { return m_desc; }

    VkImage getImage() const { return m_image; }
    VkImageView getImageView() const { return m_imageView; }

private:
    void createImageView();

    VkDevice m_device = VK_NULL_HANDLE;
    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    ASH::TextureDesc m_desc;
    bool m_ownsImage = true;
};

}