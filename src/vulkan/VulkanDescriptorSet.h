#pragma once

#include "ASH/Descriptor.h"
#include <vulkan/vulkan.h>

namespace ASH::vulkan {

class VulkanDescriptorSet final : public ASH::DescriptorSet
{
public:
    VulkanDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout);
    ~VulkanDescriptorSet() override;

    void update(const ASH::DescriptorWrite* writes, uint32_t writeCount) override;

    VkDescriptorSet getHandle() const { return m_descriptorSet; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
};

}