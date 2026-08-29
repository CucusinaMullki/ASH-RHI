#pragma once

#include "ASH/Types.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace ASH
{

class Buffer;
class Texture;
class Sampler;

//------------------
// Descriptors types
//------------------

enum class DescriptorType : uint8_t
{
    UniformBuffer,
    StorageBuffer,
    CombinedImageSampler,
    SampledImage,
    StorageImage,
    Sampler,
};

struct DescriptorBindingDesc
{
    uint32_t binding = 0;
    DescriptorType type = DescriptorType::UniformBuffer;
    uint32_t count = 1;
    ShaderStage stageFlags = ShaderStage::None;
};

struct DescriptorSetLayoutDesc
{
    std::vector<DescriptorBindingDesc> bindings;
};

class DescriptorSetLayout
{
public:
    virtual ~DescriptorSetLayout() = default;

    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

    virtual const DescriptorSetLayoutDesc& getDesc() const = 0;

protected:
    DescriptorSetLayout() = default;
};

struct DescriptorBufferInfo
{
    Buffer* buffer = nullptr;
    size_t offset = 0;
    size_t range = 0;
};

struct DescriptorImageInfo
{
    Texture* texture = nullptr;
    Sampler* sampler = nullptr;
};

struct DescriptorWrite
{
    uint32_t binding = 0;
    DescriptorType type = DescriptorType::UniformBuffer;
    uint32_t arrayElement = 0;
    const DescriptorBufferInfo* bufferInfo = nullptr;
    const DescriptorImageInfo* imageInfo = nullptr;
};

class DescriptorSet
{
public:
    virtual ~DescriptorSet() = default;

    DescriptorSet(const DescriptorSet&) = delete;
    DescriptorSet& operator=(const DescriptorSet&) = delete;

    virtual void update(const DescriptorWrite* writes, uint32_t writeCount) = 0;

protected:
    DescriptorSet() = default;
};

}