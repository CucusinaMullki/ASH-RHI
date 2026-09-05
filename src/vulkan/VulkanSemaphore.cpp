#include "VulkanSemaphore.h"
#include "VulkanResult.h"

namespace ASH::vulkan
{

VulkanSemaphore::VulkanSemaphore(VkDevice device)
    : m_device(device)
{
    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_CHECK(vkCreateSemaphore(m_device, &createInfo, nullptr, &m_semaphore), "vkCreateSemaphore");
}

VulkanSemaphore::~VulkanSemaphore()
{
    if (m_semaphore != VK_NULL_HANDLE) vkDestroySemaphore(m_device, m_semaphore, nullptr);
}

}