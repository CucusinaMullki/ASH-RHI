#pragma once

#include "ASH/Types.h"
#include <cstddef>
#include <vector>

namespace ASH
{

class RenderPass;
//--------------
// Shader module
//--------------

struct ShaderStageDesc
{
    ShaderStage stage = ShaderStage::None;
    const uint32_t* spirvCode = nullptr;
    size_t spirvSize  = 0;
    const char* entryPoint = "main";
};

//------------
//Vertex input
//------------

struct VertexAttributeDesc
{
    uint32_t location = 0;
    uint32_t binding = 0;
    Format format = Format::Unknown;
    uint32_t offset = 0;
};

struct VertexBindingDesc
{
    uint32_t binding = 0;
    uint32_t stride = 0;
    bool perInstance = false;
};

struct VertexInputDesc
{
    std::vector<VertexBindingDesc> binding;
    std::vector<VertexAttributeDesc> attributes;
};

//---------------------
// Fix functions states
//---------------------

struct RasterizationState
{
    PolygonMode polygonMode = PolygonMode::Fill;
    CullMode cullMode = CullMode::Back;
    FrontFace frontFace = FrontFace::CounterClockwise;
    float lineWidth = 1.0f;
};

struct DepthStencilState
{
    bool depthTestEnable = true;
    bool depthWriteEnable = true;
    CompareOp depthCompareOp = CompareOp::Less;
};

struct ColorBlendAttachmentState
{
    bool blendEnable = false;
    BlendFactor srcColorFactor = BlendFactor::One;
    BlendFactor dstColorFactor = BlendFactor::Zero;
    BlendOp colorBlendOp = BlendOp::Add;
    BlendFactor srcAlphaFactor = BlendFactor::One;
    BlendFactor dstAlphaFactor = BlendFactor::Zero;
    BlendOp alphaBlendOp = BlendOp::Add; 
};

struct ColorBlendState
{
    std::vector<ColorBlendAttachmentState> attachments;
};

//-----------------------
// Pipelines descriptions
//-----------------------

struct GraphicsPipelineDesc
{
    std::vector<ShaderStageDesc> stages;
    VertexInputDesc vertexInput;
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    RasterizationState rasterization;
    DepthStencilState depthStencil;
    ColorBlendState colorBlend;
    RenderPass* renderPass = nullptr;
    const char* debugName = nullptr;
};

struct ComputePipelineDesc
{
    ShaderStageDesc stage;
    const char* debugName = nullptr;
};

enum class PipelineType : uint8_t
{
    Graphics,
    Compute,
};

class Pipeline
{
public:
    virtual ~Pipeline() = default;

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    virtual PipelineType getType() const = 0;

protected:
    Pipeline() = default;
    
};

}