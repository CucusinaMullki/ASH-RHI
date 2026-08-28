#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

namespace ASH::vulkan
{
    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
}