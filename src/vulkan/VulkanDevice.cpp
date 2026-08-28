#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapChain.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanResult.h"

#include <cstdio>
#include <vector>

namespace ASH::vulkan {

namespace {

constexpr const char* kValidationLayer = "VK_LAYER_KHRONOS_validation";

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData)
{
    (void)severity;
    (void)type;
    (void)userData;
    std::fprintf(stderr, "[vulkan] %s\n", callbackData->pMessage);
    return VK_FALSE;
}

VkResult createDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* outMessenger) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    if (func == nullptr)
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    return func(instance, &createInfo, nullptr, outMessenger);
}

void destroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (func != nullptr)
    {
        func(instance, messenger, nullptr);
    }
}

bool isDiscreteGpu(const VkPhysicalDeviceProperties& props)
{
    return props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

uint32_t findGraphicsQueueFamily(VkPhysicalDevice physicalDevice)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, families.data());

    for (uint32_t i = 0; i < count; ++i)
    {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        {
            return i;
        }
    }

    throw std::runtime_error("No queue family with schedule support found.");
}

}

VulkanDevice::VulkanDevice(bool enableValidation, const std::vector<const char*>& requiredExtensions)
    : validationEnabled_(enableValidation)
{
    createInstance(enableValidation, requiredExtensions);
    selectPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
}

VulkanDevice::~VulkanDevice()
{
    if (m_device != VK_NULL_HANDLE) 
    {
        vkDeviceWaitIdle(m_device);
    }

    if (m_commandPool != VK_NULL_HANDLE) vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    if (m_device != VK_NULL_HANDLE) vkDestroyDevice(m_device, nullptr);

    if (m_debugMessenger != VK_NULL_HANDLE) destroyDebugMessenger(m_instance, m_debugMessenger);
    if (m_surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    if (m_instance != VK_NULL_HANDLE) vkDestroyInstance(m_instance, nullptr);
}

void VulkanDevice::createInstance(bool enableValidation, const std::vector<const char*>& requiredExtensions)
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "ash-app";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "ASH";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    std::vector<const char*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME };
    extensions.insert(extensions.end(), requiredExtensions.begin(), requiredExtensions.end());

    if (enableValidation)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidation)
    {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = &kValidationLayer;
    }

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance), "vkCreateInstance");

    if (enableValidation)
    {
        createDebugMessenger(m_instance, &m_debugMessenger);
    }
}

void VulkanDevice::selectPhysicalDevice() 
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw std::runtime_error("No Vulkan-supported device found.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    VkPhysicalDevice fallback = devices[0];
    for (VkPhysicalDevice candidate : devices)
    {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(candidate, &props);

        if (isDiscreteGpu(props))
        {
            m_physicalDevice = candidate;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE)
    {
        m_physicalDevice = fallback;
    }

    m_graphicsQueueFamily = findGraphicsQueueFamily(m_physicalDevice);
}

void VulkanDevice::createLogicalDevice() {
    const float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = m_graphicsQueueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkPhysicalDeviceFeatures features{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueInfo;
    createInfo.pEnabledFeatures = &features;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device), "vkCreateDevice");

    vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);
}

void VulkanDevice::createCommandPool() 
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_graphicsQueueFamily;

    VK_CHECK(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool), "vkCreateCommandPool");
}

void VulkanDevice::attachSurface(VkSurfaceKHR surface) 
{
    m_surface = surface;
}

std::unique_ptr<ASH::Buffer> VulkanDevice::createBuffer(const ASH::BufferDesc& desc) 
{
    return std::make_unique<VulkanBuffer>(m_device, m_physicalDevice, desc);
}

std::unique_ptr<ASH::Texture> VulkanDevice::createTexture(const ASH::TextureDesc& desc) 
{
    return std::make_unique<VulkanTexture>(m_device, m_physicalDevice, desc);
}

std::unique_ptr<ASH::RenderPass> VulkanDevice::createRenderPass(const ASH::RenderPassDesc& desc)
{
    return std::make_unique<VulkanRenderPass>(m_device, desc);
}

std::unique_ptr<ASH::Framebuffer> VulkanDevice::createFramebuffer(const ASH::FramebufferDesc& desc)
{
    return std::make_unique<VulkanFramebuffer>(m_device, desc);
}

std::unique_ptr<ASH::Pipeline> VulkanDevice::createGraphicsPipeline(const ASH::GraphicsPipelineDesc& desc)
{
    return std::make_unique<VulkanPipeline>(m_device, desc);
}

std::unique_ptr<ASH::Pipeline> VulkanDevice::createComputePipeline(const ASH::ComputePipelineDesc& desc)
{
    return std::make_unique<VulkanPipeline>(m_device, desc);
}

std::unique_ptr<ASH::SwapChain> VulkanDevice::createSwapChain(const ASH::SwapChainDesc& desc) {
    if (m_surface == VK_NULL_HANDLE) {
        throw std::runtime_error("createSwapChain called for attachSurface()");
    }
    return std::make_unique<VulkanSwapChain>(m_device, m_physicalDevice, m_surface, m_graphicsQueueFamily, desc);
}

std::unique_ptr<ASH::CommandBuffer> VulkanDevice::createCommandBuffer() 
{
    return std::make_unique<VulkanCommandBuffer>(m_device, m_commandPool);
}

void VulkanDevice::submit(ASH::CommandBuffer* commandBuffer) 
{
    auto* vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer);
    VkCommandBuffer handle = vulkanCommandBuffer->getHandle();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &handle;

    VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE), "vkQueueSubmit");
}

void VulkanDevice::waitIdle()
{
    vkDeviceWaitIdle(m_device);
}

}