#pragma once

#include "ASH/Types.h"
#include "ASH/Texture.h"

namespace ASH
{

struct SwapChainDesc
{
    Extent2D extent {};
    Format format = Format::B8G8R8A8_UNorm;
    uint32_t imageCount = 3;
    PresentMode presentMode = PresentMode::Fifo;
};

class SwapChain
{
public:
    virtual ~SwapChain() = default;

    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    virtual bool acquireNextImage(uint32_t& outImageIndex) = 0;

    virtual ASH::Texture* getImage(uint32_t index) = 0;
    virtual uint32_t getImageCount() const = 0;
    virtual Extent2D getExtent() const = 0;

    virtual void present(uint32_t imageIndex) = 0;
    virtual void resize(Extent2D newExtnet) = 0;

protected:
    SwapChain() = default;
};

}