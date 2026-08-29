#pragma once

#include "ASH/Sampler.h"
#include <vulkan/vulkan.h>

namespace ASH::vulkan
{

class VulkanSampler final : public ASH::Sampler
{
public:
    VulkanSampler(VkDevice device, const ASH::SamplerDesc& desc);
    ~VulkanSampler() override;

    const ASH::SamplerDesc& getDesc() const override { return m_desc; }

    VkSampler getHandle() const { return m_sampler; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
    ASH::SamplerDesc m_desc;
};

}