#pragma once

#include "ASH/Descriptor.h"
#include <vulkan/vulkan.h>

namespace ASH::vulkan
{

class VulkanDescriptorSetLayout final : public ASH::DescriptorSetLayout
{
public:
    VulkanDescriptorSetLayout(VkDevice device, const ASH::DescriptorSetLayoutDesc& desc);
    ~VulkanDescriptorSetLayout() override;

    const ASH::DescriptorSetLayoutDesc& getDesc() const override { return m_desc; }

    VkDescriptorSetLayout getHandle() const { return m_layout; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    ASH::DescriptorSetLayoutDesc m_desc;
};

}