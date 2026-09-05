# ASH RHI

**ASH** is an RHI (Render Hardware Interface) - a C++ library for comfortable work with graphics APIs (as of autumn 2026, only the Vulkan API).

## Overview

**ASH** has two layers:
1. `include/ASH/` - clean abstract interfaces (`Device`, `Buffer`, `Texture`, `Pipeline`, `Semaphore`, `Fence`, etc.) that know nothing about Vulkan.
2. `src/vulkan/` - the concrete implementation of these interfaces on top of Vulkan (`VulkanDevice`, `VulkanBuffer`, etc.).

This separation makes it possible to add another backend (e.g. D3D12) in the future without touching code that uses `ASH`. Application code obtains a `Device` through `ASH::createDevice(Backend::Vulkan, ...)` and, from that point on, works exclusively through `ASH::*` interface pointers - no `Vk*` types, no `static_cast` to backend-specific classes, no raw `vkQueueSubmit` calls, and no mention of the concrete backend class name anywhere outside the one `Backend::Vulkan` enum value.

The one deliberate exception is creating the platform window surface itself (see "Window surfaces are a deliberate exception" below) - everything else is fully abstracted.

## !! IMPORTANT: GLFW is NOT part of the RHI !!

**ASH** only abstracts the GPU (creating resources, recording commands, synchronization) - it does not create windows and should not know anything about window systems or input.

**GLFW is used only by the test app in `test/`** - as one possible example of integrating with a windowing library (you can use a different one). The RHI's only surface-related touchpoint is `Device::attachSurface(void* nativeSurfaceHandle)`, which accepts a platform-agnostic pointer that the concrete backend reinterprets as needed (a `VkSurfaceKHR` for the Vulkan backend).

## Requirements

To build the library:
- CMake >= 3.20
- A compiler with C++20 support
- Vulkan SDK (headers + loader + validation layers)

Additionally, for the test app in `test/`:
- GLFW3 (as an example windowing library)
- `glslc` (shader compiler, part of the Vulkan SDK)

## Build

```bash
cmake -B build
cmake --build build
```

Result:
- `build/src/librhi.a` - static library (`ASH::*` implementation on Vulkan), **does not depend on GLFW**.
- `build/test/ash_test` - test app (window via GLFW + a rotating textured triangle).

Run the test:
```bash
./build/test/ash_test
```

## Documentation

- **README.md** (this file) - project overview, build instructions, architectural decisions.
- **docs/USAGE.md** - step-by-step guide: from creating a `Device` to a render loop with descriptors and textures.

## Quick usage example

```cpp
#include "ASH/ASH.h"   // no backend-specific header needed at all

auto device = ASH::createDevice(ASH::Backend::Vulkan, /*enableValidation=*/true);

ASH::BufferDesc desc{};
desc.size   = 1024;
desc.usage  = ASH::BufferUsage::VertexBuffer;
desc.memory = ASH::MemoryUsage::CpuToGpu;

auto buffer = device->createBuffer(desc);
void* data = buffer->map();
// ... write data ...
buffer->unmap();
```

A full render-loop example with a window (via GLFW, as an example), descriptors and textures - see `test/main.cpp` and `docs/USAGE.md`.

## Architectural decisions

### Backend selection through a factory

`ASH::createDevice(Backend backend, bool enableValidation, const std::vector<const char*>& requiredExtensions)` is the single function in the entire library that names a concrete backend class (`ASH::vulkan::VulkanDevice`) - implemented in `src/Device.cpp`, outside the `src/vulkan/` backend directory. It returns `std::unique_ptr<ASH::Device>`; application code never needs to spell out `ASH::vulkan::VulkanDevice` itself. Adding a second backend means adding a new `case` to this one `switch` - nothing in application code changes.

### Window surfaces are a deliberate exception

Creating a window surface (`glfwCreateWindowSurface`) is inherently platform- and backend-specific - it needs a real `VkInstance` on the Vulkan backend, which has no equivalent in the abstract `Device` interface (and reasonably shouldn't: an `ASH::Instance` abstraction would exist purely to support this one call). The pragmatic choice is a single `static_cast<ASH::vulkan::VulkanDevice*>(device.get())` right at the point of surface creation, to reach `getInstance()`. Everything before and after that one line - including attaching the resulting surface via `Device::attachSurface(void*)` - goes through the abstract interface. See section 3 of `docs/USAGE.md`.

### Resource ownership

All resources are returned as `std::unique_ptr<ASH::Interface>` from `Device` factory methods. Resource lifetime is tied to the `unique_ptr` - plain RAII, no manual `destroy()`.

**Important:** resources that reference each other (`Framebuffer` holds pointers to `Texture`, `Pipeline` references `RenderPass`) must be destroyed in the reverse order of creation. See section 16 of `docs/USAGE.md` for details.

### Synchronization is fully abstracted

`ASH::Semaphore` and `ASH::Fence` are opaque interfaces, created through `Device::createSemaphore()` / `Device::createFence(bool initiallySignaled)`. Submission goes through `Device::submit(const ASH::SubmitInfo&)`, which accepts abstract `Semaphore*`/`Fence*` and a `PipelineStage` enum for the wait stage. `SwapChain::acquireNextImage`/`present` likewise take an `ASH::Semaphore*` directly, rather than an internal frame-slot index.

This means frame-in-flight management (how many semaphores, how many command buffers, how they're indexed) is entirely up to the application - `SwapChain` itself has no built-in notion of "frames in flight" and holds no synchronization primitives internally.

**Important detail:** the semaphore signaled on `present()` must be indexed **per swapchain image**, not per frame-in-flight slot - see section 14 of `docs/USAGE.md` for why, and the git history for the data race this fixed.

### Memory suballocation

`VulkanBuffer`/`VulkanTexture` do not call `vkAllocateMemory` directly - instead they go through a shared `VulkanMemoryAllocator` (one instance per `Device`), which allocates large blocks (256 MB) and carves them up with a linear allocator. This avoids hitting the GPU's limit on the number of simultaneous allocations (commonly 4096).

The current allocator does **not** reuse freed space inside a block - a block is only fully released once all its suballocations have been freed. This is fine for a "create once at load time, live long" usage pattern; a workload with frequent create/destroy churn would need a free-list allocator instead.

### Multithreading

`VulkanDevice` creates a dedicated `VkCommandPool` per thread that requests a command buffer (lazily, on first use). `vkQueueSubmit` itself is **not** protected against concurrent calls — submission is expected to happen from a single (usually the main) thread.

### Barriers

`CommandBuffer::barrier()` converts `ASH::ResourceState` into precise `VkPipelineStageFlags`/`VkAccessFlags` via a transition table (`VulkanFormat.cpp`, `toBarrierMasks`), rather than using the blunt `VK_PIPELINE_STAGE_ALL_COMMANDS_BIT`.

### Device lost

`VulkanResult.h` throws a dedicated `ASH::DeviceLostException` (a `std::runtime_error` subclass) on `VK_ERROR_DEVICE_LOST`, distinguishable from ordinary errors. `VulkanDevice::isDeviceLost()` allows proactively checking device state without waiting for an exception (reachable only through the concrete backend type, since it's not part of the abstract interface).

## Implementation status

| Feature | Status |
| --- | --- |
| Backend factory (`ASH::createDevice`) | Done |
| Device / GPU selection / queues | Done |
| Buffer / Texture | Done |
| RenderPass / Framebuffer / Pipeline | Done |
| CommandBuffer (draw/dispatch/copy/barrier) | Done |
| Semaphore / Fence / Device::submit | Done |
| SwapChain + resize | Done |
| Descriptor Sets (UBO, CombinedImageSampler) | Done |
| Sampler | Done |
| Push Constants | API is ready, but exercised only in compilation, not in a real render loop |
| Frames-in-flight (application-managed) | Done |
| Memory suballocation | Done |
| Depth Testing | Done |
| Indexed Geometry (`drawIndexed`) | API is ready, needs a scene-level test |
| Compute Pipeline + `dispatch` | API is ready, needs a scene-level test |
| Multi-attachment RenderPass | API is ready, needs a scene-level test |
| Device Lost Handling | Done |
| Multithreaded command pools | Done |

## Known limitations

- The suballocation allocator has no defragmentation (see above).
- No loading of texture/model files (PNG/glTF, etc.) - deliberately out of scope for the RHI; that's the application/asset pipeline's job.
- No mipmap generation and no compressed texture formats.
- `vkQueueSubmit` is not thread-safe by itself (see "Multithreading" above).
- One `VkCommandPool` per thread, but no pool of reusable command buffers - every `createCommandBuffer()` is a fresh allocation.
- Window surface creation requires one `static_cast` to the concrete backend type (see "Window surfaces are a deliberate exception" above) — this is intentional, not an oversight.

## Development

Build and check after every change:
```bash
g++ -std=c++20 -Wall -Wextra -fsyntax-only -Iinclude -Isrc src/vulkan/<file>.cpp
```
faster than a full rebuild, catches syntax errors before `cmake --build`.

Full rebuild from scratch (after structural CMake changes):
```bash
rm -rf build && mkdir build && cd build && cmake .. && cmake --build .
```