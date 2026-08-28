#include "VulkanBuffer.h"
#include "VulkanFormat.h"
#include "VulkanMemory.h"
#include "VulkanResult.h"

namespace ASH::vulkan
{

namespace
{
    VkMemoryPropertyFlags toMemoryProperties(ASH::MemoryUsage usage)
    {
        switch (usage)
        {
            case ASH::MemoryUsage::GpuOnly:
                return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            case ASH::MemoryUsage::CpuToGpu:
                return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            case ASH::MemoryUsage::GpuToCpu:
                return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                    | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                    | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        }
        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
}

VulkanBuffer::VulkanBuffer(VkDevice device, VkPhysicalDevice physicalDevice, const ASH::BufferDesc& desc)
    : m_device(device)
    , m_desc(desc)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.size;
    bufferInfo.usage = toVkBufferUsage(desc.usage);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_buffer), "vkCreateBuffer");

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(m_device, m_buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits,
        toMemoryProperties(desc.memory));

    VK_CHECK(vkAllocateMemory(m_device, &allocInfo, nullptr, &m_memory), "vkAllocateMemory (buffer)");

    VK_CHECK(vkBindBufferMemory(m_device, m_buffer, m_memory, 0), "vkBindBufferMemory");
}

VulkanBuffer::~VulkanBuffer() {
    if (m_buffer != VK_NULL_HANDLE) vkDestroyBuffer(m_device, m_buffer, nullptr);
    if (m_memory != VK_NULL_HANDLE) vkFreeMemory(m_device, m_memory, nullptr);
}

void* VulkanBuffer::map() {
    void* data = nullptr;
    VK_CHECK(vkMapMemory(m_device, m_memory, 0, m_desc.size, 0, &data), "vkMapMemory");
    return data;
}

void VulkanBuffer::unmap() {
    vkUnmapMemory(m_device, m_memory);
}

}