#pragma once

#include "ASH/Buffer.h"
#include "VulkanMemoryAllocator.h"
#include <vulkan/vulkan.h>

namespace ASH::vulkan
{

class VulkanBuffer final : public ASH::Buffer
{
public:
    VulkanBuffer(VkDevice device, VulkanMemoryAllocator* allocator, const ASH::BufferDesc& desc);
    ~VulkanBuffer() override;

    size_t getSize() const override { return m_desc.size; }
    ASH::BufferUsage getUsage() const override { return m_desc.usage; }

    void* map() override;
    void unmap() override;

    VkBuffer getHandle() const { return m_buffer; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VulkanMemoryAllocator* m_allocator = nullptr;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    MemoryAllocation m_allocation{};
    ASH::BufferDesc m_desc;
};

}