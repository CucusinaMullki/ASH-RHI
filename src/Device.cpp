#include "ASH/Device.h"
#include "vulkan/VulkanDevice.h"
#include <stdexcept>

namespace ASH
{

std::unique_ptr<Device> createDevice(Backend backend, bool enableValidation, const std::vector<const char*>& requiredExtensions)
{
    switch (backend)
    {
    case Backend::Vulkan:
        return std::make_unique<vulkan::VulkanDevice>(enableValidation, requiredExtensions);
    }
    throw std::runtime_error("Unknown backend");
}

}