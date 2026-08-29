#include "VulkanSampler.h"
#include "VulkanFormat.h"
#include "VulkanResult.h"

namespace ASH::vulkan {

VulkanSampler::VulkanSampler(VkDevice device, const ASH::SamplerDesc& desc)
    : m_device(device)
    , m_desc(desc)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = toVkFilter(desc.magFilter);
    samplerInfo.minFilter = toVkFilter(desc.minFilter);

    samplerInfo.addressModeU = toVkAddressMode(desc.addressModeU);
    samplerInfo.addressModeV = toVkAddressMode(desc.addressModeV);
    samplerInfo.addressModeW = toVkAddressMode(desc.addressModeW);

    const bool anisotropyRequested = desc.maxAnisotropy > 1.0f;
    samplerInfo.anisotropyEnable = anisotropyRequested ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = desc.maxAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = desc.minLod;
    samplerInfo.maxLod = desc.maxLod;
    samplerInfo.mipLodBias = 0.0f;

    VK_CHECK(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler), "vkCreateSampler");
}

VulkanSampler::~VulkanSampler() {
    if (m_sampler != VK_NULL_HANDLE) vkDestroySampler(m_device, m_sampler, nullptr);
}

}