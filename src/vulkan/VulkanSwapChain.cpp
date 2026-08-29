#include "VulkanSwapChain.h"
#include "VulkanFormat.h"
#include "VulkanResult.h"
#include <cstdint>

namespace ASH::vulkan
{

VulkanSwapChain::VulkanSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                  uint32_t queueFamily, const ASH::SwapChainDesc& desc)
    : m_device(device)
    , m_physicalDevice(physicalDevice)
    , m_surface(surface)
    , m_queueFamily(queueFamily)
    , m_desc(desc)
{
    vkGetDeviceQueue(m_device, m_queueFamily, 0, &m_presentQueue);

    createSyncObjects();
    create(desc.extent);
}

VulkanSwapChain::~VulkanSwapChain()
{
    destroySwapchainObjects();
    destroySyncObjects();
}

void VulkanSwapChain::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
    {
        VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]), "vkCreateSemaphore (imageAvailable)");
        VK_CHECK(vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]), "vkCreateFence");
    }
}

void VulkanSwapChain::destroySyncObjects() {
    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
    {
        if (m_imageAvailableSemaphores[i] != VK_NULL_HANDLE) vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        if (m_inFlightFences[i] != VK_NULL_HANDLE) vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }
}

void VulkanSwapChain::create(ASH::Extent2D extent)
{
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);

    VkExtent2D chosenExtent{ extent.width, extent.height };
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        chosenExtent = capabilities.currentExtent;
    }

    uint32_t imageCount = m_desc.imageCount;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    if (imageCount < capabilities.minImageCount)
    {
        imageCount = capabilities.minImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = toVkFormat(m_desc.format);
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = chosenExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = toVkPresentMode(m_desc.presentMode);
    createInfo.clipped = VK_TRUE;

    VK_CHECK(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain), "vkCreateSwapchainKHR");

    uint32_t actualImageCount = 0;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImageCount, nullptr);
    m_rawImages.resize(actualImageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImageCount, m_rawImages.data());

    m_extent = { chosenExtent.width, chosenExtent.height };

    ASH::TextureDesc textureDesc{};
    textureDesc.type = ASH::TextureType::Texture2D;
    textureDesc.format = m_desc.format;
    textureDesc.extent = { m_extent.width, m_extent.height, 1 };
    textureDesc.mipLevels = 1;
    textureDesc.arrayLayers = 1;
    textureDesc.usage = ASH::TextureUsage::ColorAttachment;

    m_images.reserve(m_rawImages.size());
    for (VkImage image : m_rawImages)
    {
        m_images.push_back(std::make_unique<VulkanTexture>(m_device, image, textureDesc));
    }

    m_renderFinishedSemaphores.resize(actualImageCount, VK_NULL_HANDLE);
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (uint32_t i = 0; i < actualImageCount; ++i) 
    {
        VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]), "vkCreateSemaphore (renderFinished)");
    }
}

void VulkanSwapChain::destroySwapchainObjects() 
{
    for (VkSemaphore semaphore : m_renderFinishedSemaphores)
    {
        if (semaphore != VK_NULL_HANDLE) vkDestroySemaphore(m_device, semaphore, nullptr);
    }
    m_renderFinishedSemaphores.clear();

    m_images.clear();
    m_rawImages.clear();

    if (m_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
}

void VulkanSwapChain::waitForFrame(uint32_t frameIndex)
{
    VK_CHECK(vkWaitForFences(m_device, 1, &m_inFlightFences[frameIndex], VK_TRUE, UINT64_MAX), "vkWaitForFences");
}

bool VulkanSwapChain::acquireNextImage(uint32_t& outImageIndex, uint32_t frameIndex)
{
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
        m_imageAvailableSemaphores[frameIndex], VK_NULL_HANDLE, &outImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        return false;
    }
    
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        VK_CHECK(result, "vkAcquireNextImageKHR");
    }

    VK_CHECK(vkResetFences(m_device, 1, &m_inFlightFences[frameIndex]), "vkResetFences");

    return true;
}

void VulkanSwapChain::present(uint32_t imageIndex, uint32_t frameIndex)
{
    (void)frameIndex;

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[imageIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_presentQueue, &presentInfo);
}

bool VulkanSwapChain::acquireNextImage(uint32_t& outImageIndex)
{
    return acquireNextImage(outImageIndex, 0);
}

void VulkanSwapChain::present(uint32_t imageIndex)
{
    present(imageIndex, 0);
}

ASH::Texture* VulkanSwapChain::getImage(uint32_t index)
{
    return m_images[index].get();
}

void VulkanSwapChain::resize(ASH::Extent2D newExtent)
{
    vkDeviceWaitIdle(m_device);
    destroySwapchainObjects();
    create(newExtent);
}

}