#pragma once

#include "ASH/Sync.h"
#include <vulkan/vulkan.h>

namespace ASH::vulkan
{

class VulkanSemaphore final : public ASH::Semaphore
{
public:
    explicit VulkanSemaphore(VkDevice device);
    ~VulkanSemaphore() override;

    VkSemaphore getHandle() const { return m_semaphore; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkSemaphore m_semaphore = VK_NULL_HANDLE;
};

}