#pragma once

#include <string>
#include <stdexcept>
#include <cstdint>

namespace ASH
{
//------------------------------
    //GEOMETRY / Main structures
//------------------------------

struct Extent2D
{
    uint32_t width = 0;
    uint32_t height = 0;
};

struct Extent3D
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;
};

struct Rect2D
{
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct Viewport
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float minDepth = 0.0f;
    float maxDepth = 1.0f;
};

union ClearColor 
{
    float f32[4];
    int32_t i32[4];
    uint32_t u32[4];
};

struct ClearDepthStencil
{
    float depth = 1.0f;
    uint32_t stencil = 0;
};

//-----------
    //FORMATS
//-----------

enum class Format : uint16_t
{
    Unknown = 0,

    R8_UNorm,
    R8G8_UNorm,
    R8G8B8A8_UNorm,
    R8G8B8A8_SRGB,
    B8G8R8A8_UNorm,
    B8G8R8A8_SRGB,

    R16_Float,
    R16G16_Float,
    R16G16B16A16_Float,

    R32_Float,
    R32G32_Float,
    R32G32B32_Float,
    R32G32B32A32_Float,

    D16_UNorm,
    D24_UNorm_S8_UInt,
    D32_Float,
    D32_Float_S8_UInt,
};

//------------------------
    //RESOURCE USING FLAGS
//------------------------

enum class BufferUsage : uint32_t 
{
    None = 0,
    VertexBuffer = 1u << 0,
    IndexBuffer = 1u << 1,
    UniformBuffer = 1u << 2,
    StorageBuffer = 1u << 3,
    IndirectBuffer = 1u << 4,
    TransferSrc = 1u << 5,
    TransferDst = 1u << 6,
};

constexpr BufferUsage operator|(BufferUsage a, BufferUsage b)
{
    return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

constexpr BufferUsage operator&(BufferUsage a, BufferUsage b)
{
    return static_cast<BufferUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

constexpr bool hasFlag(BufferUsage value, BufferUsage flag)
{
    return (value & flag) == flag;
}

enum class TextureUsage : uint32_t
{
    None = 0,
    Sampled = 1u << 0,
    Storage = 1u << 1,
    ColorAttachment = 1u << 2,
    DepthStencilAttachment = 1u << 3,
    TransferSrc = 1u << 4,
    TransferDst = 1u << 5,
};

constexpr TextureUsage operator|(TextureUsage a, TextureUsage b)
{
    return static_cast<TextureUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

constexpr TextureUsage operator&(TextureUsage a, TextureUsage b)
{
    return static_cast<TextureUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

constexpr bool hasFlag(TextureUsage value, TextureUsage flag)
{
    return (value & flag) == flag;
}

enum class ShaderStage : uint32_t
{
    None        = 0,
    Vertex      = 1u << 0,
    Fragment    = 1u << 1,
    Compute     = 1u << 2,
    Geometry    = 1u << 3,
    TessControl = 1u << 4,
    TessEval    = 1u << 5,
};

constexpr ShaderStage operator|(ShaderStage a, ShaderStage b)
{
    return static_cast<ShaderStage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

constexpr ShaderStage operator&(ShaderStage a, ShaderStage b)
{
    return static_cast<ShaderStage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
constexpr bool hasFlag(ShaderStage value, ShaderStage flag)
{
    return (value & flag) == flag;
}

enum class MemoryUsage : uint8_t
{
    GpuOnly,
    CpuToGpu,
    GpuToCpu,
};

//-------------------
    //PIPELINE STATES
//-------------------

enum class TextureType : uint8_t
{
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCube,
};

enum class PrimitiveTopology : uint8_t
{
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
};

enum class PolygonMode : uint8_t
{
    Fill,
    Line,
    Point,
};

enum class CullMode : uint8_t
{
    None,
    Front,
    Back,
    FrontAndBack,
};

enum class FrontFace : uint8_t {
    CounterClockwise,
    Clockwise,
};

enum class CompareOp : uint8_t {
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always,
};

enum class BlendFactor : uint8_t {
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstColor,
    OneMinusDstColor,
    DstAlpha,
    OneMinusDstAlpha,
};

enum class BlendOp : uint8_t {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

enum class LoadOp : uint8_t {
    Load,
    Clear,
    DontCare,
};

enum class StoreOp : uint8_t {
    Store,
    DontCare,
};

enum class IndexType : uint8_t {
    UInt16,
    UInt32,
};

enum class Filter : uint8_t {
    Nearest,
    Linear,
};

enum class AddressMode : uint8_t {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
};

enum class PresentMode : uint8_t {
    Immediate,
    Fifo,
    Mailbox,
};

enum class ResourceState : uint8_t {
    Undefined,
    ColorAttachment,
    DepthStencilAttachment,
    ShaderReadOnly,
    TransferSrc,
    TransferDst,
    Present,
};

class DeviceLostException : public std::runtime_error
{
public:
    explicit DeviceLostException(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

}