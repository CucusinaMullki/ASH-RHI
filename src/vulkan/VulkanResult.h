#pragma once

#include "ASH/Types.h"
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <string>

namespace ASH::vulkan
{
    inline void vkCheck(VkResult result, const char* what)
    {
        if (result != VK_SUCCESS) 
        {
            throw std::runtime_error(std::string("Vulkan error [") + std::to_string(result) + "]: " + what);
        }

        if (result == VK_ERROR_DEVICE_LOST)
        {
            throw ASH::DeviceLostException(
                std::string("Vulkan device lost during: ") + what
            );
        }
    }
}

#define VK_CHECK(expr, what) ::ASH::vulkan::vkCheck((expr), (what))