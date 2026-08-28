#pragma once

#include "ASH/Types.h"
#include <vector>

namespace ASH
{

struct AttachmentDesc
{
    Format format = Format::Unknown;
    uint32_t sampleCount = 1;
    LoadOp loadOp = LoadOp::Clear;
    StoreOp storeOp = StoreOp::Store;
    LoadOp stencilLoadOp = LoadOp::DontCare;
    StoreOp stencilStoreOp = StoreOp::DontCare;
};

struct RenderPassDesc
{
    std::vector<AttachmentDesc> colorAttachment;
    bool hasDepthStencil = false;
    AttachmentDesc depthStencilAttachment;
};

class RenderPass
{
public:
    virtual ~RenderPass() = default;

    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;
    
    virtual const RenderPassDesc& getDesc() const = 0;

protected:
    RenderPass() = default;
};

}