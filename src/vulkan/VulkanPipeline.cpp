#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanFormat.h"
#include "VulkanResult.h"
#include <vector>

namespace ASH::vulkan
{

namespace
{

VkShaderModule createShaderModule(VkDevice device, const ASH::ShaderStageDesc& stage)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = stage.spirvSize;
    createInfo.pCode = stage.spirvCode;

    VkShaderModule module = VK_NULL_HANDLE;
    VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &module), "vkCreateShaderModule");
    return module;
}

VkPipelineLayout createEmptyPipelineLayout(VkDevice device)
{
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkPipelineLayout layout = VK_NULL_HANDLE;
    VK_CHECK(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout), "vkCreatePipelineLayout");
    return layout;
}

}

VulkanPipeline::VulkanPipeline(VkDevice device, const ASH::GraphicsPipelineDesc& desc)
    : m_device(device)
    , m_type(ASH::PipelineType::Graphics)
{
    std::vector<VkShaderModule> shaderModules;
    std::vector<VkPipelineShaderStageCreateInfo> stageInfos;
    shaderModules.reserve(desc.stages.size());
    stageInfos.reserve(desc.stages.size());

    for (const auto& stage : desc.stages)
    {
        VkShaderModule module = createShaderModule(m_device, stage);
        shaderModules.push_back(module);

        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = toVkShaderStage(stage.stage);
        stageInfo.module = module;
        stageInfo.pName = stage.entryPoint;
        stageInfos.push_back(stageInfo);
    }

    std::vector<VkVertexInputBindingDescription> bindings;
    bindings.reserve(desc.vertexInput.binding.size());
    for (const auto& binding : desc.vertexInput.binding)
    {
        VkVertexInputBindingDescription b{};
        b.binding = binding.binding;
        b.stride = binding.stride;

        b.inputRate = binding.perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
        bindings.push_back(b);
    }

    std::vector<VkVertexInputAttributeDescription> attributes;
    attributes.reserve(desc.vertexInput.attributes.size());
    for (const auto& attribute : desc.vertexInput.attributes)
    {
        VkVertexInputAttributeDescription a{};
        a.location = attribute.location;
        a.binding = attribute.binding;
        a.format = toVkFormat(attribute.format);
        a.offset = attribute.offset;
        attributes.push_back(a);
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
    vertexInputInfo.pVertexBindingDescriptions = bindings.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = toVkPrimitiveTopology(desc.topology);

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = toVkPolygonMode(desc.rasterization.polygonMode);
    rasterizer.cullMode    = toVkCullMode(desc.rasterization.cullMode);
    rasterizer.frontFace   = toVkFrontFace(desc.rasterization.frontFace);
    rasterizer.lineWidth   = desc.rasterization.lineWidth;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = desc.depthStencil.depthTestEnable ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = desc.depthStencil.depthWriteEnable ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = toVkCompareOp(desc.depthStencil.depthCompareOp);

    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;
    blendAttachments.reserve(desc.colorBlend.attachments.size());
    for (const auto& attachment : desc.colorBlend.attachments)
    {
        VkPipelineColorBlendAttachmentState state{};
        state.blendEnable = attachment.blendEnable ? VK_TRUE : VK_FALSE;
        state.srcColorBlendFactor = toVkBlendFactor(attachment.srcColorFactor);
        state.dstColorBlendFactor = toVkBlendFactor(attachment.dstColorFactor);
        state.colorBlendOp = toVkBlendOp(attachment.colorBlendOp);
        state.srcAlphaBlendFactor = toVkBlendFactor(attachment.srcAlphaFactor);
        state.dstAlphaBlendFactor = toVkBlendFactor(attachment.dstAlphaFactor);
        state.alphaBlendOp = toVkBlendOp(attachment.alphaBlendOp);
        state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachments.push_back(state);
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = static_cast<uint32_t>(blendAttachments.size());
    colorBlending.pAttachments = blendAttachments.data();

    const VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    m_layout = createEmptyPipelineLayout(m_device);

    auto* vulkanRenderPass = static_cast<VulkanRenderPass*>(desc.renderPass);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(stageInfos.size());
    pipelineInfo.pStages = stageInfos.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_layout;
    pipelineInfo.renderPass = vulkanRenderPass->getHandle();
    pipelineInfo.subpass = 0;

    VK_CHECK(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline),
        "vkCreateGraphicsPipelines");

    for (VkShaderModule module : shaderModules)
    {
        vkDestroyShaderModule(m_device, module, nullptr);
    }
}

VulkanPipeline::VulkanPipeline(VkDevice device, const ASH::ComputePipelineDesc& desc)
    : m_device(device)
    , m_type(ASH::PipelineType::Compute)
{

    VkShaderModule module = createShaderModule(m_device, desc.stage);

    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;

    stageInfo.module = module;
    stageInfo.pName = desc.stage.entryPoint;

    m_layout = createEmptyPipelineLayout(m_device);

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = stageInfo;
    pipelineInfo.layout = m_layout;

    VK_CHECK(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline),
        "vkCreateComputePipelines");

    vkDestroyShaderModule(m_device, module, nullptr);
}

VulkanPipeline::~VulkanPipeline()
{
    if (m_pipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_device, m_pipeline, nullptr);
    if (m_layout   != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_device, m_layout, nullptr);
}

}