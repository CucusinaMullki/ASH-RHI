#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <memory>
#include <vector>

namespace ASH::vulkan
{

struct MemoryAllocation
{
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
    uint32_t blockIndex = 0;
};

class VulkanMemoryAllocator
{
public:
    VulkanMemoryAllocator(VkDevice device, VkPhysicalDevice physicalDevice);
    ~VulkanMemoryAllocator();

    VulkanMemoryAllocator(const VulkanMemoryAllocator&) = delete;
    VulkanMemoryAllocator& operator=(const VulkanMemoryAllocator&) = delete;

    MemoryAllocation allocate(const VkMemoryRequirements& requirements, VkMemoryPropertyFlags properties);
    void free(const MemoryAllocation& allocation);

    void* getMappedPointer(const MemoryAllocation& allocation);
    
private:
    static constexpr VkDeviceSize kBlockSize = 256ull * 1024 * 1024;

    struct Block {
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize size = 0;
        VkDeviceSize cursor = 0;
        uint32_t memoryTypeIndex = 0;
        void* mappedPointer = nullptr;
        uint32_t liveAllocationCount = 0;
    };

    Block& findOrCreateBlock(uint32_t memoryTypeIndex, VkDeviceSize requiredSize, VkMemoryPropertyFlags properties);

    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    std::vector<Block> m_blocks;
};

}