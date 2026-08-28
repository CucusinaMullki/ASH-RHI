#include "VulkanMemory.h"
#include <stdexcept>

namespace ASH::vulkan
{

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        const bool typeSupported = (typeFilter & (1u << i)) != 0;

        const bool hasProperties = (memProperties.memoryTypes[i].propertyFlags & properties) == properties;

        if (typeSupported && hasProperties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find correct Vulkan memory type");
}

}