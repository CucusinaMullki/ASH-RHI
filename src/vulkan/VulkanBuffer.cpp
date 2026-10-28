#include "VulkanBuffer.h"
#include "VulkanFormat.h"
#include "VulkanResult.h"

namespace ASH::vulkan {

namespace {

VkMemoryPropertyFlags toMemoryProperties(ASH::MemoryUsage usage) {
    switch (usage) {
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

uint64_t VulkanBuffer::getDeviceAddress() const
{
    const auto u = m_desc.usage;
    const bool needsAddress = 
        (u & ASH::BufferUsage::ShaderDeviceAddress) != ASH::BufferUsage::None ||
        (u & ASH::BufferUsage::AccelerationStructureStorage) != ASH::BufferUsage::None ||
        (u & ASH::BufferUsage::AccelerationStructureInput) != ASH::BufferUsage::None ||
        (u & ASH::BufferUsage::ShaderBindingTable) != ASH::BufferUsage::None;
    if (!needsAddress) return 0;

    VkBufferDeviceAddressInfo info{};
    info.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    info.buffer = m_buffer;
    return vkGetBufferDeviceAddress(m_device, &info);
}

VulkanBuffer::VulkanBuffer(VkDevice device, VulkanMemoryAllocator* allocator, const ASH::BufferDesc& desc)
    : m_device(device)
    , m_allocator(allocator)
    , m_desc(desc)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size        = desc.size;
    bufferInfo.usage       = toVkBufferUsage(desc.usage);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_buffer), "vkCreateBuffer");

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(m_device, m_buffer, &memRequirements);

    m_allocation = m_allocator->allocate(memRequirements, toMemoryProperties(desc.memory));

    VK_CHECK(vkBindBufferMemory(m_device, m_buffer, m_allocation.memory, m_allocation.offset), "vkBindBufferMemory");
}

VulkanBuffer::~VulkanBuffer() {
    if (m_buffer != VK_NULL_HANDLE) vkDestroyBuffer(m_device, m_buffer, nullptr);
    m_allocator->free(m_allocation);
}

void* VulkanBuffer::map()
{
    void* ptr = m_allocator->getMappedPointer(m_allocation);
    if (ptr == nullptr)
    {
        throw std::runtime_error("VulkanBuffer::map() вызван для GPU-only памяти (не HOST_VISIBLE)");
    }
    return ptr;
}

void VulkanBuffer::unmap()
{

}

}