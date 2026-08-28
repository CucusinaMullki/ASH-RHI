#pragma once

#include "ASH/Pipeline.h"
#include <vulkan/vulkan.h>

namespace ASH::vulkan
{

class VulkanPipeline final : public ASH::Pipeline
{
public:
    VulkanPipeline(VkDevice device, const ASH::GraphicsPipelineDesc& desc);
    VulkanPipeline(VkDevice device, const ASH::ComputePipelineDesc& desc);
    ~VulkanPipeline() override;

    ASH::PipelineType getType() const override { return m_type; }

    VkPipeline getHandle() const { return m_pipeline; }
    VkPipelineLayout getLayout() const { return m_layout; }
    
private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    ASH::PipelineType m_type;
};

}