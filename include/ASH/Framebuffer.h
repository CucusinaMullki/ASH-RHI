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
    Texture* depthStencilAttachment = nullptr;
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