#include "VulkanMemoryAllocator.h"
#include "VulkanMemory.h"
#include "VulkanResult.h"
#include <stdexcept>

namespace ASH::vulkan
{

VulkanMemoryAllocator::VulkanMemoryAllocator(VkDevice device, VkPhysicalDevice physicalDevice)
    : m_device(device)
    , m_physicalDevice(physicalDevice)
{
}

VulkanMemoryAllocator::~VulkanMemoryAllocator()
{
    for (Block& block : m_blocks)
    {
        if (block.mappedPointer != nullptr)
        {
            vkUnmapMemory(m_device, block.memory);
        }
        if (block.memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(m_device, block.memory, nullptr);
        }
    }
}

VulkanMemoryAllocator::Block& VulkanMemoryAllocator::findOrCreateBlock(
    uint32_t memoryTypeIndex, VkDeviceSize requiredSize, VkMemoryPropertyFlags properties)
{
    for (Block& block : m_blocks)
    {
        if (block.memoryTypeIndex == memoryTypeIndex && (block.size - block.cursor) >= requiredSize)
        {
            return block;
        }
    }

    VkDeviceSize blockSize = requiredSize > kBlockSize ? requiredSize : kBlockSize;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = blockSize;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory memory = VK_NULL_HANDLE;
    VK_CHECK(vkAllocateMemory(m_device, &allocInfo, nullptr, &memory), "vkAllocateMemory (block)");

    Block newBlock{};
    newBlock.memory = memory;
    newBlock.size = blockSize;
    newBlock.cursor = 0;
    newBlock.memoryTypeIndex = memoryTypeIndex;
    newBlock.liveAllocationCount = 0;

    if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        VK_CHECK(vkMapMemory(m_device, memory, 0, blockSize, 0, &newBlock.mappedPointer), "vkMapMemory (block)");
    }

    m_blocks.push_back(newBlock);
    return m_blocks.back();
}

MemoryAllocation VulkanMemoryAllocator::allocate(const VkMemoryRequirements& requirements, VkMemoryPropertyFlags properties)
{
    uint32_t memoryTypeIndex = findMemoryType(m_physicalDevice, requirements.memoryTypeBits, properties);

    Block& block = findOrCreateBlock(memoryTypeIndex, requirements.size, properties);

    VkDeviceSize alignedOffset = (block.cursor + requirements.alignment - 1) & ~(requirements.alignment - 1);

    if (alignedOffset + requirements.size > block.size)
    {
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        VkDeviceMemory memory = VK_NULL_HANDLE;
        VK_CHECK(vkAllocateMemory(m_device, &allocInfo, nullptr, &memory), "vkAllocateMemory (oversized block)");

        Block newBlock{};
        newBlock.memory = memory;
        newBlock.size = requirements.size;
        newBlock.cursor = requirements.size;
        newBlock.memoryTypeIndex = memoryTypeIndex;
        newBlock.liveAllocationCount = 1;

        if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            VK_CHECK(vkMapMemory(m_device, memory, 0, requirements.size, 0, &newBlock.mappedPointer), "vkMapMemory (oversized block)");
        }

        m_blocks.push_back(newBlock);

        MemoryAllocation allocation{};
        allocation.memory = memory;
        allocation.offset = 0;
        allocation.size = requirements.size;
        allocation.blockIndex = static_cast<uint32_t>(m_blocks.size() - 1);
        return allocation;
    }

    block.cursor = alignedOffset + requirements.size;
    block.liveAllocationCount += 1;

    MemoryAllocation allocation{};
    allocation.memory = block.memory;
    allocation.offset = alignedOffset;
    allocation.size = requirements.size;
    allocation.blockIndex = static_cast<uint32_t>(&block - &m_blocks[0]);

    return allocation;
}

void VulkanMemoryAllocator::free(const MemoryAllocation& allocation)
{
    Block& block = m_blocks[allocation.blockIndex];
    block.liveAllocationCount -= 1;

    if (block.liveAllocationCount == 0)
    {
        if (block.mappedPointer != nullptr)
        {
            vkUnmapMemory(m_device, block.memory);
            block.mappedPointer = nullptr;
        }
        vkFreeMemory(m_device, block.memory, nullptr);
        block.memory = VK_NULL_HANDLE;
        block.cursor = 0;
    }
}

void* VulkanMemoryAllocator::getMappedPointer(const MemoryAllocation& allocation)
{
    Block& block = m_blocks[allocation.blockIndex];
    if (block.mappedPointer == nullptr)
    {
        return nullptr;
    }
    return static_cast<uint8_t*>(block.mappedPointer) + allocation.offset;
}

}