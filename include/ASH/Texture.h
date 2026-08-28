#pragma once

#include "ASH/Types.h"

namespace ASH {

struct TextureDesc {
    TextureType  type        = TextureType::Texture2D;
    Format       format      = Format::Unknown;
    Extent3D     extent      {};
    uint32_t     mipLevels   = 1;
    uint32_t     arrayLayers = 1;
    uint32_t     sampleCount = 1;
    TextureUsage usage       = TextureUsage::None;
    const char*  debugName   = nullptr;
};

class Texture {
public:
    virtual ~Texture() = default;

    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;

    virtual const TextureDesc& getDesc() const = 0;

protected:
    Texture() = default;
};

}