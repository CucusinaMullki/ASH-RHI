#pragma once

#include "ASH/SwapChain.h"
#include "VulkanTexture.h"
#include "VulkanSemaphore.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace ASH::vulkan
{

class VulkanSwapChain final : public ASH::SwapChain
{
public:
    VulkanSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
        uint32_t queueFamily, const ASH::SwapChainDesc& desc);
    ~VulkanSwapChain() override;

    bool acquireNextImage(uint32_t& outImageIndex, ASH::Semaphore* signalSemaphore) override;

    ASH::Texture* getImage(uint32_t index) override;
    uint32_t getImageCount() const override { return static_cast<uint32_t>(m_images.size()); }
    ASH::Extent2D getExtent() const override { return m_extent; }

    void present(uint32_t imageIndex, ASH::Semaphore* waitSemaphore) override;
    void resize(ASH::Extent2D newExtent) override;

private:
    void create(ASH::Extent2D extent);
    void destroySwapchainObjects();

    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    uint32_t m_queueFamily = 0;

    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    ASH::SwapChainDesc m_desc;
    ASH::Extent2D m_extent{};

    std::vector<VkImage> m_rawImages;
    std::vector<std::unique_ptr<VulkanTexture>> m_images;
};

}