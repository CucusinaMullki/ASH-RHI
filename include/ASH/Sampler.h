#pragma once

#include "ASH/Types.h"

namespace ASH
{

struct SamplerDesc
{
    Filter magFilter = Filter::Linear;
    Filter minFilter = Filter::Linear;
    AddressMode addressModeU = AddressMode::Repeat;
    AddressMode addressModeV = AddressMode::Repeat;
    AddressMode addressModeW = AddressMode::Repeat;
    float maxAnisotropy = 1.0f;
    float minLod = 0.0f;
    float maxLod = 1000.0f;
    const char* debugName = nullptr;
};

class Sampler {
public:
    virtual ~Sampler() = default;

    Sampler(const Sampler&) = delete;
    Sampler& operator=(const Sampler&) = delete;

    virtual const SamplerDesc& getDesc() const = 0;

protected:
    Sampler() = default;
};

}