#pragma once

#include "ASH/Sync.h"
#include <vulkan/vulkan.h>
#include <cstdint>

namespace ASH::vulkan
{

class VulkanFence final : public ASH::Fence
{
public:
    VulkanFence(VkDevice device, bool initiallySignaled);
    ~VulkanFence() override;

    void wait(uint64_t timeoutNs) override;
    void reset() override;

    VkFence getHandle() const { return m_fence; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;
};

}