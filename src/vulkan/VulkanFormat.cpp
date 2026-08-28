#include "VulkanFormat.h"

namespace ASH::vulkan {

VkFormat toVkFormat(ASH::Format format) {
    switch (format) {
        case ASH::Format::R8_UNorm: return VK_FORMAT_R8_UNORM;
        case ASH::Format::R8G8_UNorm: return VK_FORMAT_R8G8_UNORM;
        case ASH::Format::R8G8B8A8_UNorm: return VK_FORMAT_R8G8B8A8_UNORM;
        case ASH::Format::R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
        case ASH::Format::B8G8R8A8_UNorm: return VK_FORMAT_B8G8R8A8_UNORM;
        case ASH::Format::B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
        case ASH::Format::R16_Float: return VK_FORMAT_R16_SFLOAT;
        case ASH::Format::R16G16_Float: return VK_FORMAT_R16G16_SFLOAT;
        case ASH::Format::R16G16B16A16_Float: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case ASH::Format::R32_Float: return VK_FORMAT_R32_SFLOAT;
        case ASH::Format::R32G32_Float: return VK_FORMAT_R32G32_SFLOAT;
        case ASH::Format::R32G32B32_Float: return VK_FORMAT_R32G32B32_SFLOAT;
        case ASH::Format::R32G32B32A32_Float: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case ASH::Format::D16_UNorm: return VK_FORMAT_D16_UNORM;
        case ASH::Format::D24_UNorm_S8_UInt: return VK_FORMAT_D24_UNORM_S8_UINT;
        case ASH::Format::D32_Float: return VK_FORMAT_D32_SFLOAT;
        case ASH::Format::D32_Float_S8_UInt: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case ASH::Format::Unknown: return VK_FORMAT_UNDEFINED;
    }
    return VK_FORMAT_UNDEFINED;
}

VkBufferUsageFlags toVkBufferUsage(ASH::BufferUsage usage)
{
    VkBufferUsageFlags flags = 0;

    if (hasFlag(usage, ASH::BufferUsage::VertexBuffer)) flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (hasFlag(usage, ASH::BufferUsage::IndexBuffer)) flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (hasFlag(usage, ASH::BufferUsage::UniformBuffer)) flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (hasFlag(usage, ASH::BufferUsage::StorageBuffer)) flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (hasFlag(usage, ASH::BufferUsage::IndirectBuffer)) flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    if (hasFlag(usage, ASH::BufferUsage::TransferSrc)) flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (hasFlag(usage, ASH::BufferUsage::TransferDst)) flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    return flags;
}

VkImageUsageFlags toVkImageUsage(ASH::TextureUsage usage)
{
    VkImageUsageFlags flags = 0;

    if (hasFlag(usage, ASH::TextureUsage::Sampled)) flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (hasFlag(usage, ASH::TextureUsage::Storage)) flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (hasFlag(usage, ASH::TextureUsage::ColorAttachment)) flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (hasFlag(usage, ASH::TextureUsage::DepthStencilAttachment)) flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (hasFlag(usage, ASH::TextureUsage::TransferSrc)) flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (hasFlag(usage, ASH::TextureUsage::TransferDst)) flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    return flags;
}

VkImageType toVkImageType(ASH::TextureType type) {
    switch (type) {
        case ASH::TextureType::Texture1D: return VK_IMAGE_TYPE_1D;
        case ASH::TextureType::Texture2D: return VK_IMAGE_TYPE_2D;
        case ASH::TextureType::Texture3D: return VK_IMAGE_TYPE_3D;
        case ASH::TextureType::TextureCube: return VK_IMAGE_TYPE_2D;
    }
    return VK_IMAGE_TYPE_2D;
}

VkImageViewType toVkImageViewType(ASH::TextureType type) {
    switch (type) {
        case ASH::TextureType::Texture1D: return VK_IMAGE_VIEW_TYPE_1D;
        case ASH::TextureType::Texture2D: return VK_IMAGE_VIEW_TYPE_2D;
        case ASH::TextureType::Texture3D: return VK_IMAGE_VIEW_TYPE_3D;
        case ASH::TextureType::TextureCube: return VK_IMAGE_VIEW_TYPE_CUBE;
    }
    return VK_IMAGE_VIEW_TYPE_2D;
}

VkShaderStageFlagBits toVkShaderStage(ASH::ShaderStage stage)
{
    switch (stage)
    {
        case ASH::ShaderStage::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
        case ASH::ShaderStage::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ASH::ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
        case ASH::ShaderStage::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ASH::ShaderStage::TessControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ASH::ShaderStage::TessEval: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ASH::ShaderStage::None: return VK_SHADER_STAGE_ALL;
    }
    return VK_SHADER_STAGE_ALL;
}

VkPrimitiveTopology toVkPrimitiveTopology(ASH::PrimitiveTopology topology)
{
    switch (topology)
    {
        case ASH::PrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case ASH::PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case ASH::PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case ASH::PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case ASH::PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    }
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

VkPolygonMode toVkPolygonMode(ASH::PolygonMode mode)
{
    switch (mode)
    {
        case ASH::PolygonMode::Fill: return VK_POLYGON_MODE_FILL;
        case ASH::PolygonMode::Line: return VK_POLYGON_MODE_LINE;
        case ASH::PolygonMode::Point: return VK_POLYGON_MODE_POINT;
    }
    return VK_POLYGON_MODE_FILL;
}

VkCullModeFlags toVkCullMode(ASH::CullMode mode)
{
    switch (mode)
    {
        case ASH::CullMode::None: return VK_CULL_MODE_NONE;
        case ASH::CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
        case ASH::CullMode::Back: return VK_CULL_MODE_BACK_BIT;
        case ASH::CullMode::FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
    }
    return VK_CULL_MODE_BACK_BIT;
}

VkFrontFace toVkFrontFace(ASH::FrontFace face)
{
    switch (face)
    {
        case ASH::FrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case ASH::FrontFace::Clockwise: return VK_FRONT_FACE_CLOCKWISE;
    }
    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

VkCompareOp toVkCompareOp(ASH::CompareOp op)
{
    switch (op) {
        case ASH::CompareOp::Never: return VK_COMPARE_OP_NEVER;
        case ASH::CompareOp::Less: return VK_COMPARE_OP_LESS;
        case ASH::CompareOp::Equal: return VK_COMPARE_OP_EQUAL;
        case ASH::CompareOp::LessOrEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case ASH::CompareOp::Greater: return VK_COMPARE_OP_GREATER;
        case ASH::CompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
        case ASH::CompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case ASH::CompareOp::Always: return VK_COMPARE_OP_ALWAYS;
    }
    return VK_COMPARE_OP_LESS;
}

VkBlendFactor toVkBlendFactor(ASH::BlendFactor factor)
{
    switch (factor)
    {
        case ASH::BlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
        case ASH::BlendFactor::One: return VK_BLEND_FACTOR_ONE;
        case ASH::BlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
        case ASH::BlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case ASH::BlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
        case ASH::BlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case ASH::BlendFactor::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
        case ASH::BlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case ASH::BlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
        case ASH::BlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    }
    return VK_BLEND_FACTOR_ONE;
}

VkBlendOp toVkBlendOp(ASH::BlendOp op)
{
    switch (op)
    {
        case ASH::BlendOp::Add: return VK_BLEND_OP_ADD;
        case ASH::BlendOp::Subtract: return VK_BLEND_OP_SUBTRACT;
        case ASH::BlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case ASH::BlendOp::Min: return VK_BLEND_OP_MIN;
        case ASH::BlendOp::Max: return VK_BLEND_OP_MAX;
    }
    return VK_BLEND_OP_ADD;
}

VkAttachmentLoadOp toVkLoadOp(ASH::LoadOp op)
{
    switch (op)
    {
        case ASH::LoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
        case ASH::LoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case ASH::LoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

VkAttachmentStoreOp toVkStoreOp(ASH::StoreOp op)
{
    switch (op)
    {
        case ASH::StoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
        case ASH::StoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

VkIndexType toVkIndexType(ASH::IndexType type) 
{
    switch (type)
    {
        case ASH::IndexType::UInt16: return VK_INDEX_TYPE_UINT16;
        case ASH::IndexType::UInt32: return VK_INDEX_TYPE_UINT32;
    }
    return VK_INDEX_TYPE_UINT32;
}

VkFilter toVkFilter(ASH::Filter filter) {
    switch (filter)
    {
        case ASH::Filter::Nearest: return VK_FILTER_NEAREST;
        case ASH::Filter::Linear: return VK_FILTER_LINEAR;
    }
    return VK_FILTER_LINEAR;
}

VkSamplerAddressMode toVkAddressMode(ASH::AddressMode mode)
{
    switch (mode)
    {
        case ASH::AddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case ASH::AddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case ASH::AddressMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case ASH::AddressMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

VkPresentModeKHR toVkPresentMode(ASH::PresentMode mode)
{
    switch (mode)
    {
        case ASH::PresentMode::Immediate: return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case ASH::PresentMode::Fifo: return VK_PRESENT_MODE_FIFO_KHR;
        case ASH::PresentMode::Mailbox:  return VK_PRESENT_MODE_MAILBOX_KHR;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkImageLayout toVkImageLayout(ASH::ResourceState state)
{
    switch (state)
    {
        case ASH::ResourceState::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
        case ASH::ResourceState::ColorAttachment: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ASH::ResourceState::DepthStencilAttachment: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ASH::ResourceState::ShaderReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case ASH::ResourceState::TransferSrc: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ASH::ResourceState::TransferDst: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ASH::ResourceState::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

bool isDepthFormat(ASH::Format format)
{
    switch (format)
    {
        case ASH::Format::D16_UNorm:
        case ASH::Format::D24_UNorm_S8_UInt:
        case ASH::Format::D32_Float:
        case ASH::Format::D32_Float_S8_UInt:
            return true;
        default:
            return false;
    }
}

bool isStencilFormat(ASH::Format format)
{
    switch (format)
    {
        case ASH::Format::D24_UNorm_S8_UInt:
        case ASH::Format::D32_Float_S8_UInt:
            return true;
        default:
            return false;
    }
}

}