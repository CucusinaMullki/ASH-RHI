#pragma once

#include "ASH/Device.h"
#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>

namespace ASH::vulkan
{

class VulkanDevice final : public ASH::Device
{
public:
    VulkanDevice(bool enableValidation, const std::vector<const char*>& requiredExtensions = {});
    ~VulkanDevice() override;

    std::unique_ptr<ASH::Buffer>        createBuffer(const ASH::BufferDesc& desc) override;
    std::unique_ptr<ASH::Texture>       createTexture(const ASH::TextureDesc& desc) override;
    std::unique_ptr<ASH::RenderPass>    createRenderPass(const ASH::RenderPassDesc& desc) override;
    std::unique_ptr<ASH::Framebuffer>   createFramebuffer(const ASH::FramebufferDesc& desc) override;
    std::unique_ptr<ASH::Pipeline>      createGraphicsPipeline(const ASH::GraphicsPipelineDesc& desc) override;
    std::unique_ptr<ASH::Pipeline>      createComputePipeline(const ASH::ComputePipelineDesc& desc) override;
    std::unique_ptr<ASH::SwapChain>     createSwapChain(const ASH::SwapChainDesc& desc) override;
    std::unique_ptr<ASH::CommandBuffer> createCommandBuffer() override;
    
    void submit(ASH::CommandBuffer* commandBuffer) override;
    void waitIdle() override;

    void attachSurface(VkSurfaceKHR surface);

    VkInstance       getInstance()           const { return m_instance; }
    VkPhysicalDevice getPhysicalDevice()      const { return m_physicalDevice; }
    VkDevice         getHandle()              const { return m_device; }
    VkQueue          getGraphicsQueue()       const { return m_graphicsQueue; }
    uint32_t         getGraphicsQueueFamily() const { return m_graphicsQueueFamily; }
    VkCommandPool    getCommandPool()         const { return m_commandPool; }

private:
    void createInstance(bool enableValidation, const std::vector<const char*>& requiredExtensions);
    void selectPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice         m_physicalDevice = VK_NULL_HANDLE;
    VkDevice                 m_device = VK_NULL_HANDLE;
    VkQueue                  m_graphicsQueue = VK_NULL_HANDLE;
    uint32_t                 m_graphicsQueueFamily = 0;
    VkCommandPool            m_commandPool = VK_NULL_HANDLE;
    VkSurfaceKHR             m_surface = VK_NULL_HANDLE;

    bool validationEnabled_ = false;
};

}