#pragma once

#include <memory>

namespace ASH
{

struct BufferDesc;
struct TextureDesc;
struct RenderPassDesc;
struct FramebufferDesc;
struct GraphicsPipelineDesc;
struct ComputePipelineDesc;
struct SwapChainDesc;

class Buffer;
class Texture;
class RenderPass;
class Framebuffer;
class Pipeline;
class SwapChain;
class CommandBuffer;

struct DescriptorSetLayoutDesc;
struct SamplerDesc;

class DescriptorSetLayout;
class DescriptorSet;
class Sampler;

class Device
{
public:
    virtual ~Device() = default;

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    virtual std::unique_ptr<Buffer> createBuffer(const BufferDesc& desc) = 0;
    virtual std::unique_ptr<Texture> createTexture(const TextureDesc& desc) = 0;
    virtual std::unique_ptr<RenderPass> createRenderPass(const RenderPassDesc& desc) = 0;
    virtual std::unique_ptr<Framebuffer> createFramebuffer(const FramebufferDesc& desc) = 0;
    virtual std::unique_ptr<Pipeline> createGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;
    virtual std::unique_ptr<Pipeline> createComputePipeline(const ComputePipelineDesc& desc) = 0;
    virtual std::unique_ptr<SwapChain> createSwapChain(const SwapChainDesc& desc) =0 ;
    virtual std::unique_ptr<CommandBuffer> createCommandBuffer() = 0;
    virtual std::unique_ptr<DescriptorSetLayout> createDescriptorSetLayout(const DescriptorSetLayoutDesc& desc) = 0;
    virtual std::unique_ptr<DescriptorSet> createDescriptorSet(DescriptorSetLayout* layout) = 0;
    virtual std::unique_ptr<Sampler> createSampler(const SamplerDesc& desc) = 0;

    virtual void submit(CommandBuffer* CommandBuffer) = 0;
    virtual void waitIdle() = 0;

protected:
    Device() = default;
};

}