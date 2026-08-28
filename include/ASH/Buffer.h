#pragma once

#include "ASH/Types.h"
#include <cstddef>

namespace ASH
{

struct BufferDesc 
{
    size_t size = 0;
    BufferUsage usage = BufferUsage::None;
    MemoryUsage memory = MemoryUsage::GpuOnly;
    const char* debugName = nullptr;
};

class Buffer
{
public:
    virtual ~Buffer() = default;

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    virtual size_t getSize() const = 0;
    virtual BufferUsage getUsage() const = 0;

    virtual void* map() = 0;
    virtual void unmap() = 0;

protected:
    Buffer() = default;
};

}