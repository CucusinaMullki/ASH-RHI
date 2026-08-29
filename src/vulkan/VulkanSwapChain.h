#pragma once

#include "ASH/SwapChain.h"
#include "VulkanTexture.h"
#include <vulkan/vulkan.h>
#include <array>
#include <memory>
#include <vector>

namespace ASH::vulkan
{

class VulkanSwapChain final : public ASH::SwapChain
{
public:
    static constexpr uint32_t kMaxFramesInFlight = 2;

    VulkanSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
        uint32_t queueFamily, const ASH::SwapChainDesc& desc);
    ~VulkanSwapChain() override;

    bool acquireNextImage(uint32_t& outImageIndex) override;
    void present(uint32_t imageIndex) override;

    ASH::Texture* getImage(uint32_t index) override;
    uint32_t getImageCount() const override { return static_cast<uint32_t>(m_images.size()); }
    ASH::Extent2D getExtent() const override { return m_extent; }

    void resize(ASH::Extent2D newExtent) override;

    void waitForFrame(uint32_t frameIndex);
    bool acquireNextImage(uint32_t& outImageIndex, uint32_t frameIndex);

    void present(uint32_t imageIndex, uint32_t frameIndex);

    VkSemaphore getImageAvailableSemaphore(uint32_t frameIndex) const { return m_imageAvailableSemaphores[frameIndex]; }
    VkSemaphore getRenderFinishedSemaphore(uint32_t imageIndex) const { return m_renderFinishedSemaphores[imageIndex]; }
    VkFence getInFlightFence(uint32_t frameIndex) const { return m_inFlightFences[frameIndex]; }

private:
    void create(ASH::Extent2D extent);
    void destroySwapchainObjects();
    void createSyncObjects();
    void destroySyncObjects();

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

    std::array<VkSemaphore, kMaxFramesInFlight> m_imageAvailableSemaphores{};
    std::array<VkFence, kMaxFramesInFlight> m_inFlightFences{};

    std::vector<VkSemaphore> m_renderFinishedSemaphores;
};

}