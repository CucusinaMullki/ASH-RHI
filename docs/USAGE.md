# ASH Usage Guide

Step-by-step guide: how to create a window, a device, and draw your first triangle from scratch. Every step explains not only "what to write" but also "why it's done this way".

## Contents

- [1. Minimal dependencies](#1-minimal-dependencies)
- [2. Device — the starting point](#2-device--the-starting-point)
- [3. Window and Surface](#3-window-and-surface)
- [4. SwapChain](#4-swapchain)
- [5. Shaders: from GLSL to GPU](#5-shaders-from-glsl-to-gpu)
- [6. RenderPass and Framebuffer](#6-renderpass-and-framebuffer)
- [7. Pipeline](#7-pipeline)
- [8. Render loop](#8-render-loop)
- [9. Passing data to a shader: UBO](#9-passing-data-to-a-shader-ubo)
- [10. Textures](#10-textures)
- [11. Push Constants](#11-push-constants)
- [12. Compute Pipeline](#12-compute-pipeline)
- [13. Frames-in-flight](#13-frames-in-flight)
- [14. Window resize](#14-window-resize)
- [15. Destruction order](#15-destruction-order)

## 1. Minimal dependencies

```cpp
#include "Vulkan/VulkanDevice.h"
#include "ASH/ASH.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
```

`ASH` abstracts the **GPU**, not the window — GLFW (or any other windowing library) handles the window and input; that is not the RHI's job.

## 2. Device — the starting point

Everything starts with `Device` — the factory for every other resource.

```cpp
uint32_t extCount = 0;
const char** exts = glfwGetRequiredInstanceExtensions(&extCount);
std::vector<const char*> requiredExtensions(exts, exts + extCount);

ASH::vulkan::VulkanDevice device(/*enableValidation=*/true, requiredExtensions);
```

- `enableValidation` — keep it `true` during development. It enables Vulkan validation layers, which catch API misuse (an incorrect resource layout, a forgotten barrier, etc.) and print it straight to the console.
- `requiredExtensions` — platform-specific Vulkan extensions needed to create a surface (GLFW knows which ones it needs). If you're rendering without a window, pass an empty list.

## 3. Window and Surface

```cpp
glfwInit();
glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // don't create an OpenGL context
GLFWwindow* window = glfwCreateWindow(800, 600, "My App", nullptr, nullptr);

VkSurfaceKHR surface = VK_NULL_HANDLE;
VkResult result = glfwCreateWindowSurface(device.getInstance(), window, nullptr, &surface);
if (result != VK_SUCCESS)
{
    throw std::runtime_error("Failed to create window surface");
}
device.attachSurface(surface);
```

`attachSurface` is a method on `VulkanDevice` specifically (not part of the abstract `ASH::Device` interface), because "window surface" is a Vulkan-specific concept and is not part of the general RHI contract.

## 4. SwapChain

A chain of images that are shown on screen in turn.

```cpp
ASH::SwapChainDesc desc{};
desc.extent      = { 800, 600 };
desc.format      = ASH::Format::B8G8R8A8_UNorm;
desc.imageCount   = 3;
desc.presentMode = ASH::PresentMode::Fifo;

auto swapChain = device.createSwapChain(desc);

// Needed to access Vulkan-specific methods (semaphores, resize):
auto* vulkanSwapChain = static_cast<ASH::vulkan::VulkanSwapChain*>(swapChain.get());
```

## 5. Shaders: from GLSL to GPU

Write a shader in GLSL (`.vert`/`.frag`), compile it ahead of time with `glslc` (usually wired up in CMake via `add_custom_command`) into `.spv`, then read that compiled file in C++:

```cpp
std::vector<uint32_t> readSpirvFile(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("Failed to open: " + path);

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));   // SPIR-V is a stream of 32-bit words

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));
    return buffer;
}
```

Minimal `triangle.vert`:
```glsl
#version 450
vec2 positions[3] = vec2[](vec2(0.0, -0.5), vec2(0.5, 0.5), vec2(-0.5, 0.5));
void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
```

Minimal `triangle.frag`:
```glsl
#version 450
layout(location = 0) out vec4 outColor;
void main() { outColor = vec4(1.0, 0.5, 0.2, 1.0); }
```

## 6. RenderPass and Framebuffer

**RenderPass** is a "plan": which attachments are used and how to load/store them. **Framebuffer** is the concrete set of textures bound to that plan. A Framebuffer is created **once per swapchain image** (not every frame) and is only recreated on resize.

```cpp
ASH::AttachmentDesc colorAttachment{};
colorAttachment.format = desc.format;
colorAttachment.loadOp = ASH::LoadOp::Clear;
colorAttachment.storeOp = ASH::StoreOp::Store;

ASH::RenderPassDesc rpDesc{};
rpDesc.colorAttachments = { colorAttachment };
auto renderPass = device.createRenderPass(rpDesc);

std::vector<std::unique_ptr<ASH::Framebuffer>> framebuffers;
for (uint32_t i = 0; i < swapChain->getImageCount(); ++i)
{
    ASH::FramebufferDesc fbDesc{};
    fbDesc.renderPass = renderPass.get();
    fbDesc.colorAttachments = { swapChain->getImage(i) };
    fbDesc.extent = swapChain->getExtent();
    framebuffers.push_back(device.createFramebuffer(fbDesc));
}
```

For depth testing, additionally set `rpDesc.hasDepthStencil = true` + `rpDesc.depthStencilAttachment`, create a depth texture (`ASH::Format::D32_Float`, `usage = DepthStencilAttachment`), and pass it as `fbDesc.depthStencilAttachment` in every Framebuffer.

## 7. Pipeline

Fixes the GPU pipeline state **once**: shaders, rasterization, blending, descriptor layouts. Unlike OpenGL, state does not change on the fly between draw calls — only viewport/scissor remain dynamic.

```cpp
ASH::ShaderStageDesc vertStage{};
vertStage.stage = ASH::ShaderStage::Vertex;
vertStage.spirvCode = vertCode.data();
vertStage.spirvSize = vertCode.size() * sizeof(uint32_t);

ASH::GraphicsPipelineDesc pipelineDesc{};
pipelineDesc.stages = { vertStage, fragStage };
pipelineDesc.rasterization.cullMode = ASH::CullMode::None;   // note: the default is Back
pipelineDesc.depthStencil.depthTestEnable = false;

ASH::ColorBlendAttachmentState blend{};
pipelineDesc.colorBlend.attachments = { blend };
pipelineDesc.renderPass = renderPass.get();

auto pipeline = device.createGraphicsPipeline(pipelineDesc);
```

**About culling:** the default value of `rasterization.cullMode` is `CullMode::Back`. If your triangle doesn't show up on screen for no obvious reason, this is the first thing to check — the geometry may be getting culled due to incorrect vertex winding order.

## 8. Render loop

```cpp
auto commandBuffer = device.createCommandBuffer();

while (!glfwWindowShouldClose(window))
{
    glfwPollEvents();

    uint32_t imageIndex = 0;
    if (!swapChain->acquireNextImage(imageIndex))
    {
        continue;   // swapchain is out of date — see the resize section
    }

    commandBuffer->begin();

    ASH::ClearColor clearColor{{0.02f, 0.02f, 0.05f, 1.0f}};
    ASH::RenderPassBeginInfo rpBegin{};
    rpBegin.renderPass = renderPass.get();
    rpBegin.framebuffer = framebuffers[imageIndex].get();
    rpBegin.renderArea = { 0, 0, swapChain->getExtent().width, swapChain->getExtent().height };
    rpBegin.colorClearValues = &clearColor;
    rpBegin.colorClearCount = 1;

    commandBuffer->beginRenderPass(rpBegin);
    commandBuffer->bindPipeline(pipeline.get());
    commandBuffer->setViewport(viewport);
    commandBuffer->setScissor(scissor);
    commandBuffer->draw(3, 1, 0, 0);
    commandBuffer->endRenderPass();

    // A transition to Present is required before showing the image on screen
    ASH::TextureBarrier presentBarrier{ swapChain->getImage(imageIndex),
        ASH::ResourceState::ColorAttachment,
        ASH::ResourceState::Present };
    commandBuffer->barrier(&presentBarrier, 1, nullptr, 0);
    commandBuffer->end();

    /* Submit directly through the Vulkan handle: Device::submit() does not
       support the semaphores required for swapchain presentation */
    auto* vulkanCmd = static_cast<ASH::vulkan::VulkanCommandBuffer*>(commandBuffer.get());
    VkCommandBuffer cmdHandle = vulkanCmd->getHandle();
    VkSemaphore waitSem = vulkanSwapChain->getImageAvailableSemaphore(0);
    VkSemaphore signalSem = vulkanSwapChain->getRenderFinishedSemaphore(imageIndex);
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &waitSem;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdHandle;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSem;
    vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

    swapChain->present(imageIndex);
    device.waitIdle();   // simple synchronization; see section 13 for a faster approach
}
device.waitIdle();
```

This is the simplest approach — one command buffer, `waitIdle()` every frame. It works correctly but is not the fastest option (see section 13).

## 9. Passing data to a shader: UBO

A Uniform Buffer is the standard way to send a shader data that changes rarely (e.g. once per frame): camera matrices, time, material parameters.

```cpp
// Create the buffer (once)
ASH::BufferDesc uboDesc{};
uboDesc.size = sizeof(MyUniformData);
uboDesc.usage = ASH::BufferUsage::UniformBuffer;
uboDesc.memory = ASH::MemoryUsage::CpuToGpu;   // the CPU writes to it directly
auto uniformBuffer = device.createBuffer(uboDesc);

// Layout — the "shape" of the resource set (once)
ASH::DescriptorBindingDesc binding{};
binding.binding = 0;
binding.type = ASH::DescriptorType::UniformBuffer;
binding.stageFlags = ASH::ShaderStage::Vertex;
auto layout = device.createDescriptorSetLayout({ {binding} });

// Set — the real reference to the buffer (once)
auto descriptorSet = device.createDescriptorSet(layout.get());
ASH::DescriptorBufferInfo bufferInfo{ uniformBuffer.get(), /*offset=*/0, /*range=*/0 };
ASH::DescriptorWrite write{ /*binding=*/0, ASH::DescriptorType::UniformBuffer, /*arrayElement=*/0, &bufferInfo, nullptr };
descriptorSet->update(&write, 1);

// In the Pipeline — attach the layout
pipelineDesc.descriptorSetLayouts = { layout.get() };

// In the render loop — update the data and bind
void* data = uniformBuffer->map();
std::memcpy(data, &myData, sizeof(MyUniformData));
uniformBuffer->unmap();
commandBuffer->bindDescriptorSet(pipeline.get(), 0, descriptorSet.get());
```

In the shader:
```glsl
layout(binding = 0) uniform UniformBufferObject { mat4 rotation; } ubo;
```

## 10. Textures

GPU textures **cannot** be `map()`'d directly — data is loaded through an intermediate staging buffer.

```cpp
// 1. Create the texture
ASH::TextureDesc texDesc{};
texDesc.format = ASH::Format::R8G8B8A8_UNorm;
texDesc.extent = { width, height, 1 };
texDesc.usage = ASH::TextureUsage::Sampled | ASH::TextureUsage::TransferDst;
auto texture = device.createTexture(texDesc);

// 2. Staging buffer — copy the pixels there from the CPU
ASH::BufferDesc stagingDesc{};
stagingDesc.size = pixelDataSize;
stagingDesc.usage = ASH::BufferUsage::TransferSrc;
stagingDesc.memory = ASH::MemoryUsage::CpuToGpu;
auto staging = device.createBuffer(stagingDesc);
void* p = staging->map();
std::memcpy(p, pixels, pixelDataSize);
staging->unmap();

// 3. One-off CommandBuffer: barrier -> copy -> barrier
auto cmd = device.createCommandBuffer();
cmd->begin();

ASH::TextureBarrier toDst{ texture.get(), ASH::ResourceState::Undefined, ASH::ResourceState::TransferDst };
cmd->barrier(&toDst, 1, nullptr, 0);
cmd->copyBufferToTexture(staging.get(), texture.get());
ASH::TextureBarrier toRead{ texture.get(), ASH::ResourceState::TransferDst, ASH::ResourceState::ShaderReadOnly };
cmd->barrier(&toRead, 1, nullptr, 0);

cmd->end();
// submit + waitIdle (as in section 8), then the texture is ready to use

// 4. Sampler
ASH::SamplerDesc samplerDesc{};
samplerDesc.magFilter = ASH::Filter::Linear;
samplerDesc.minFilter = ASH::Filter::Linear;
auto sampler = device.createSampler(samplerDesc);
```

For a texture descriptor, use `DescriptorType::CombinedImageSampler` and fill `write.imageInfo` (`{texture.get(), sampler.get()}`) instead of `bufferInfo`.

## 11. Push Constants

A lighter alternative to UBOs for small data (a single matrix, a few floats) that changes **every draw call** rather than once per frame. Doesn't require a separate buffer/descriptor — the data is copied directly into the command buffer.

```cpp
// In the Pipeline — declare a range
ASH::PushConstantRange range{};
range.stageFlags = ASH::ShaderStage::Vertex;
range.offset = 0;
range.size = sizeof(float);
pipelineDesc.pushConstantRanges = { range };

// In the render loop — before each draw
float value = 3.14f;
commandBuffer->pushConstants(pipeline.get(), ASH::ShaderStage::Vertex, 0, sizeof(float), &value);
commandBuffer->draw(...);
```

In the shader:
```glsl
layout(push_constant) uniform PushConstants { float value; } pc;
```

## 12. Compute Pipeline

For computation outside the graphics pipeline (physics, data processing) — doesn't draw anything, just runs a shader program over a buffer/texture.

```cpp
ASH::ComputePipelineDesc computeDesc{};
computeDesc.stage = computeShaderStage;   // ShaderStage::Compute
computeDesc.descriptorSetLayouts = { layout.get() };
auto computePipeline = device.createComputePipeline(computeDesc);

// In the render loop
commandBuffer->bindPipeline(computePipeline.get());
commandBuffer->bindDescriptorSet(computePipeline.get(), 0, computeDescriptorSet.get());
commandBuffer->dispatch(groupCountX, groupCountY, groupCountZ);
```

## 13. Frames-in-flight

Section 8 uses `waitIdle()` every frame — simple, but not the fastest approach. For CPU/GPU parallelism you need several independent slots:

```cpp
constexpr uint32_t kMaxFramesInFlight = ASH::vulkan::VulkanSwapChain::kMaxFramesInFlight;   // usually 2

std::vector<std::unique_ptr<ASH::CommandBuffer>> commandBuffers;
for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
{
    commandBuffers.push_back(device.createCommandBuffer());
}
uint32_t currentFrame = 0;

// In the loop:
vulkanSwapChain->waitForFrame(currentFrame);
uint32_t imageIndex;
vulkanSwapChain->acquireNextImage(imageIndex, currentFrame);
// ... record into commandBuffers[currentFrame] ...

/* CRITICAL: the presentation semaphore is indexed by imageIndex, NOT
   currentFrame — otherwise there is a data race when the presentation
   engine reuses the semaphore. */
VkSemaphore waitSem   = vulkanSwapChain->getImageAvailableSemaphore(currentFrame);
VkSemaphore signalSem = vulkanSwapChain->getRenderFinishedSemaphore(imageIndex);

vkQueueSubmit(..., vulkanSwapChain->getInFlightFence(currentFrame));
vulkanSwapChain->present(imageIndex, currentFrame);

currentFrame = (currentFrame + 1) % kMaxFramesInFlight;
```

No `device.waitIdle()` inside the loop — only once, after exiting it.

## 14. Window resize

```cpp
auto recreateSwapchainResources = [&]() {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    while (w == 0 || h == 0) { glfwGetFramebufferSize(window, &w, &h); glfwWaitEvents(); }

    device.waitIdle();
    vulkanSwapChain->resize({ (uint32_t)w, (uint32_t)h });

    /* Framebuffers hold references to the OLD textures — recreate everything
       that depends on the swapchain (including the depth texture, if used) */
    framebuffers.clear();
    for (uint32_t i = 0; i < swapChain->getImageCount(); ++i)
    {
        // ... device.createFramebuffer(...) again ...
    }
};

// In the render loop:
if (!swapChain->acquireNextImage(imageIndex))
{
    recreateSwapchainResources();
    continue;
}
```

## 15. Destruction order

All `ASH::*` objects are `std::unique_ptr` and are destroyed automatically (RAII) in reverse order of declaration. Rule of thumb: **an object referenced by other objects must be declared earlier and destroyed later**.

```cpp
{   // either try{} or a plain block { } — what matters is having a scope
    ASH::vulkan::VulkanDevice device(...);
    auto swapChain = device.createSwapChain(...);
    auto renderPass = device.createRenderPass(...);
    auto framebuffers = ...;   // references renderPass and swapChain
    auto pipeline = device.createGraphicsPipeline(...);   // references renderPass

    // ... render loop ...

    device.waitIdle();   // wait for the GPU before destroying anything
}   // <-- everything is destroyed HERE, in reverse order: pipeline, framebuffers,
    //     renderPass, swapChain, device

glfwDestroyWindow(window);   // ONLY after the scope above has closed
glfwTerminate();
```

**Common mistake:** calling `glfwTerminate()` before the Vulkan objects are destroyed causes a segfault inside `vkDestroySurfaceKHR`, because GLFW has already closed the connection to the window system that this call needs.