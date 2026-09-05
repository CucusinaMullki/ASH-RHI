#include "VulkanFence.h"
#include "VulkanResult.h"

namespace ASH::vulkan
{

VulkanFence::VulkanFence(VkDevice device, bool initiallySignaled)
    : m_device(device)
{
    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.flags = initiallySignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    VK_CHECK(vkCreateFence(m_device, &createInfo, nullptr, &m_fence), "vkCreateFence");
}

VulkanFence::~VulkanFence()
{
    if (m_fence != VK_NULL_HANDLE) vkDestroyFence(m_device, m_fence, nullptr);
}

void VulkanFence::wait(uint64_t timeoutNs)
{
    VK_CHECK(vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, timeoutNs), "vkWaitForFences");
}

void VulkanFence::reset()
{
    VK_CHECK(vkResetFences(m_device, 1, &m_fence), "vkResetFences");
}

}