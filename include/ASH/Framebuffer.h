#pragma once

#include "ASH/Types.h"
#include <vector>

namespace ASH
{

class RenderPass;
class Texture;

struct FramebufferDesc
{
    RenderPass* renderPass = nullptr;
    std::vector<Texture*> colorAttachments;
    std::vector<uint32_t> colorAttachmentLayers;
    std::vector<uint32_t> colorAttachmentMips;
    Texture* depthStencilAttachment = nullptr;
    uint32_t depthStencilAttachmentLayer = 0;
    uint32_t depthStencilAttachmentMip = 0;
    Extent2D extent{};
    uint32_t layers = 1;
};

class Framebuffer
{
public:
    virtual ~Framebuffer() = default;

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    virtual Extent2D getExtent() const = 0;

protected:
    Framebuffer() = default;
};

}