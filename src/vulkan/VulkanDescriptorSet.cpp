#include "VulkanDescriptorSet.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanSampler.h"
#include "VulkanResult.h"
#include <vector>

namespace ASH::vulkan {

namespace {

VkDescriptorType toVkDescriptorType(ASH::DescriptorType type) {
    switch (type) {
        case ASH::DescriptorType::UniformBuffer:        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case ASH::DescriptorType::StorageBuffer:        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case ASH::DescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case ASH::DescriptorType::SampledImage:         return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case ASH::DescriptorType::StorageImage:         return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case ASH::DescriptorType::Sampler:              return VK_DESCRIPTOR_TYPE_SAMPLER;
    }
    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

}

VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout)
    : m_device(device)
    , m_pool(pool)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VK_CHECK(vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSet), "vkAllocateDescriptorSets");
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    if (m_descriptorSet != VK_NULL_HANDLE)
    {
        vkFreeDescriptorSets(m_device, m_pool, 1, &m_descriptorSet);
    }
}

void VulkanDescriptorSet::update(const ASH::DescriptorWrite* writes, uint32_t writeCount)
{
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkDescriptorImageInfo> imageInfos;
    bufferInfos.reserve(writeCount);
    imageInfos.reserve(writeCount);

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.reserve(writeCount);

    for (uint32_t i = 0; i < writeCount; ++i)
    {
        const ASH::DescriptorWrite& write = writes[i];

        VkWriteDescriptorSet vkWrite{};
        vkWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWrite.dstSet = m_descriptorSet;
        vkWrite.dstBinding = write.binding;
        vkWrite.dstArrayElement = write.arrayElement;
        vkWrite.descriptorCount = 1;
        vkWrite.descriptorType = toVkDescriptorType(write.type);

        if (write.bufferInfo != nullptr)
        {
            auto* vulkanBuffer = static_cast<VulkanBuffer*>(write.bufferInfo->buffer);

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = vulkanBuffer->getHandle();
            bufferInfo.offset = write.bufferInfo->offset;
            bufferInfo.range  = (write.bufferInfo->range == 0) ? VK_WHOLE_SIZE : write.bufferInfo->range;

            bufferInfos.push_back(bufferInfo);
            vkWrite.pBufferInfo = &bufferInfos.back();
        }

        if (write.imageInfo != nullptr)
        {
            auto* vulkanTexture = static_cast<VulkanTexture*>(write.imageInfo->texture);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = vulkanTexture->getImageView();

            if (write.imageInfo->sampler != nullptr)
            {
                auto* vulkanSampler = static_cast<VulkanSampler*>(write.imageInfo->sampler);
                imageInfo.sampler = vulkanSampler->getHandle();
            }

            imageInfos.push_back(imageInfo);
            vkWrite.pImageInfo = &imageInfos.back();
        }

        descriptorWrites.push_back(vkWrite);
    }

    vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

}