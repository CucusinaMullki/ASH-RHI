#pragma once

#include "ASH/Device.h"
#include "VulkanMemoryAllocator.h"
#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace ASH::vulkan
{

class VulkanDevice final : public ASH::Device
{
public:
    VulkanDevice(bool enableValidation, const std::vector<const char*>& requiredExtensions = {});
    ~VulkanDevice() override;

    std::unique_ptr<ASH::Buffer> createBuffer(const ASH::BufferDesc& desc) override;
    std::unique_ptr<ASH::Texture> createTexture(const ASH::TextureDesc& desc) override;
    std::unique_ptr<ASH::RenderPass>  createRenderPass(const ASH::RenderPassDesc& desc) override;
    std::unique_ptr<ASH::Framebuffer> createFramebuffer(const ASH::FramebufferDesc& desc) override;
    std::unique_ptr<ASH::Pipeline> createGraphicsPipeline(const ASH::GraphicsPipelineDesc& desc) override;
    std::unique_ptr<ASH::Pipeline> createComputePipeline(const ASH::ComputePipelineDesc& desc) override;
    std::unique_ptr<ASH::SwapChain>  createSwapChain(const ASH::SwapChainDesc& desc) override;
    std::unique_ptr<ASH::CommandBuffer> createCommandBuffer() override;
    std::unique_ptr<ASH::Sampler> createSampler(const ASH::SamplerDesc& desc) override;
    std::unique_ptr<ASH::DescriptorSetLayout> createDescriptorSetLayout(const ASH::DescriptorSetLayoutDesc& desc) override;
    std::unique_ptr<ASH::DescriptorSet> createDescriptorSet(ASH::DescriptorSetLayout* layout) override;
    std::unique_ptr<ASH::Semaphore> createSemaphore() override;
    std::unique_ptr<ASH::Fence> createFence(bool initiallySignaled) override;

    bool isDeviceLost() const;
    
    void submit(const ASH::SubmitInfo& info) override;
    void waitIdle() override;

    void attachSurface(VkSurfaceKHR surface);

    void attachSurface(void* nativeSurfaceHandle) override { attachSurface(reinterpret_cast<VkSurfaceKHR>(nativeSurfaceHandle)); }

    VkInstance getInstance() const { return m_instance; }
    VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
    VkDevice getHandle() const { return m_device; }
    VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
    uint32_t getGraphicsQueueFamily() const { return m_graphicsQueueFamily; }
    VkDescriptorPool getDescriptorPool() const { return m_descriptorPool; }
    VulkanMemoryAllocator* getMemoryAllocator() const { return m_memoryAllocator.get(); }

private:
    void createInstance(bool enableValidation, const std::vector<const char*>& requiredExtensions);
    void selectPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();
    void createDescriptorPool();

    VkCommandPool getOrCreateCommandPool();

    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = 0;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkFence m_deviceLostCheckFence = VK_NULL_HANDLE;

    std::mutex m_commandPoolMutex;
    std::unordered_map<std::thread::id, VkCommandPool> m_commandPools;

    std::unique_ptr<VulkanMemoryAllocator> m_memoryAllocator;
    
    bool validationEnabled_ = false;
};

}