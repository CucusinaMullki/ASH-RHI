#pragma once
#include <cstdint>

namespace ASH
{

class Semaphore
{
public:
    virtual ~Semaphore() = default;

    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

protected:
    Semaphore() = default;
};

class Fence
{
public:
    virtual ~Fence() = default;

    Fence(const Fence&) = delete;
    Fence& operator=(const Fence&) = delete;

    virtual void wait(uint64_t timeoutNs = UINT64_MAX) = 0;

    virtual void reset() = 0;

protected:
    Fence() = default;
};

}