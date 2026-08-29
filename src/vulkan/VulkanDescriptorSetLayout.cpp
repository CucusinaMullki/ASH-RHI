#include "VulkanDescriptorSetLayout.h"
#include "VulkanFormat.h"
#include "VulkanResult.h"
#include <vector>

namespace ASH::vulkan {

namespace {

    VkDescriptorType toVkDescriptorType(ASH::DescriptorType type) {
    switch (type) {
        case ASH::DescriptorType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case ASH::DescriptorType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case ASH::DescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case ASH::DescriptorType::SampledImage: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case ASH::DescriptorType::StorageImage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case ASH::DescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
    }
    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device, const ASH::DescriptorSetLayoutDesc& desc)
    : m_device(device)
    , m_desc(desc)
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(desc.bindings.size());

    for (const auto& binding : desc.bindings) {
        VkDescriptorSetLayoutBinding vkBinding{};
        vkBinding.binding            = binding.binding;
        vkBinding.descriptorType     = toVkDescriptorType(binding.type);
        vkBinding.descriptorCount    = binding.count;

        VkShaderStageFlags stageFlags = 0;
        if (hasFlag(binding.stageFlags, ASH::ShaderStage::Vertex)) stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        if (hasFlag(binding.stageFlags, ASH::ShaderStage::Fragment)) stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if (hasFlag(binding.stageFlags, ASH::ShaderStage::Compute)) stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
        if (hasFlag(binding.stageFlags, ASH::ShaderStage::Geometry)) stageFlags |= VK_SHADER_STAGE_GEOMETRY_BIT;
        if (hasFlag(binding.stageFlags, ASH::ShaderStage::TessControl)) stageFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        if (hasFlag(binding.stageFlags, ASH::ShaderStage::TessEval)) stageFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        vkBinding.stageFlags = stageFlags;

        bindings.push_back(vkBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_layout), "vkCreateDescriptorSetLayout");
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
    if (m_layout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
}

}