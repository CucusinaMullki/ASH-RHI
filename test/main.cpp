#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanSwapChain.h"
#include "vulkan/VulkanCommandBuffer.h"
#include "ASH/ASH.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <string>
#include <vector>

namespace {

std::vector<uint32_t> readSpirvFile(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));

    return buffer;
}

struct Mat4 {
    float m[16];
};

Mat4 rotationZ(float angleRadians) {
    float c = std::cos(angleRadians);
    float s = std::sin(angleRadians);

    // clang-format off
    return Mat4{{
         c,    s,    0.0f, 0.0f,
        -s,    c,    0.0f, 0.0f,
         0.0f, 0.0f, 1.0f, 0.0f,
         0.0f, 0.0f, 0.0f, 1.0f
    }};
    // clang-format on
}

// Простая процедурная текстура — шахматная доска, RGBA8.
std::vector<uint8_t> generateCheckerboardPixels(uint32_t width, uint32_t height) {
    std::vector<uint8_t> pixels(width * height * 4);

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            bool isWhite = ((x / 4) + (y / 4)) % 2 == 0;

            size_t index = (y * width + x) * 4;
            pixels[index + 0] = isWhite ? 255 : 40;
            pixels[index + 1] = isWhite ? 255 : 40;
            pixels[index + 2] = isWhite ? 255 : 200;
            pixels[index + 3] = 255;
        }
    }

    return pixels;
}

} // namespace

int main() {
    if (!glfwInit()) {
        std::fprintf(stderr, "glfwInit failed\n");
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "ASH Texture Test", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        std::fprintf(stderr, "glfwCreateWindow failed\n");
        return 1;
    }

    int exitCode = 0;

    try {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        ASH::vulkan::VulkanDevice device(true, requiredExtensions);
        std::printf("VulkanDevice created.\n");

        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkResult surfaceResult = glfwCreateWindowSurface(device.getInstance(), window, nullptr, &surface);
        if (surfaceResult != VK_SUCCESS) {
            std::fprintf(stderr, "glfwCreateWindowSurface returned VkResult: %d\n", surfaceResult);
            throw std::runtime_error("glfwCreateWindowSurface failed");
        }
        device.attachSurface(surface);
        std::printf("Surface attached.\n");

        // --- SwapChain ---
        ASH::SwapChainDesc swapChainDesc{};
        swapChainDesc.extent      = { 800, 600 };
        swapChainDesc.format      = ASH::Format::B8G8R8A8_UNorm;
        swapChainDesc.imageCount  = 3;
        swapChainDesc.presentMode = ASH::PresentMode::Fifo;

        auto swapChain = device.createSwapChain(swapChainDesc);
        auto* vulkanSwapChain = static_cast<ASH::vulkan::VulkanSwapChain*>(swapChain.get());

        std::printf("SwapChain created. Image count: %u, extent: %ux%u\n",
                    swapChain->getImageCount(),
                    swapChain->getExtent().width,
                    swapChain->getExtent().height);

        // --- RenderPass ---
        ASH::AttachmentDesc colorAttachment{};
        colorAttachment.format         = swapChainDesc.format;
        colorAttachment.sampleCount    = 1;
        colorAttachment.loadOp         = ASH::LoadOp::Clear;
        colorAttachment.storeOp        = ASH::StoreOp::Store;
        colorAttachment.stencilLoadOp  = ASH::LoadOp::DontCare;
        colorAttachment.stencilStoreOp = ASH::StoreOp::DontCare;

        ASH::RenderPassDesc renderPassDesc{};
        renderPassDesc.colorAttachments = { colorAttachment };
        renderPassDesc.hasDepthStencil  = false;

        auto renderPass = device.createRenderPass(renderPassDesc);
        std::printf("RenderPass created.\n");

        // --- Framebuffers ---
        std::vector<std::unique_ptr<ASH::Framebuffer>> framebuffers;
        framebuffers.reserve(swapChain->getImageCount());
        for (uint32_t i = 0; i < swapChain->getImageCount(); ++i) {
            ASH::FramebufferDesc fbDesc{};
            fbDesc.renderPass             = renderPass.get();
            fbDesc.colorAttachments       = { swapChain->getImage(i) };
            fbDesc.depthStencilAttachment = nullptr;
            fbDesc.extent                 = swapChain->getExtent();
            fbDesc.layers                 = 1;
            framebuffers.push_back(device.createFramebuffer(fbDesc));
        }
        std::printf("Framebuffers created: %zu\n", framebuffers.size());

        // --- Пересоздание swapchain-зависимых ресурсов (resize / OUT_OF_DATE) ---
        auto recreateSwapchainResources = [&]() {
            int width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            while (width == 0 || height == 0) {
                glfwGetFramebufferSize(window, &width, &height);
                glfwWaitEvents();
            }
            device.waitIdle();
            vulkanSwapChain->resize({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });

            framebuffers.clear();
            framebuffers.reserve(swapChain->getImageCount());
            for (uint32_t i = 0; i < swapChain->getImageCount(); ++i) {
                ASH::FramebufferDesc fbDesc{};
                fbDesc.renderPass             = renderPass.get();
                fbDesc.colorAttachments       = { swapChain->getImage(i) };
                fbDesc.depthStencilAttachment = nullptr;
                fbDesc.extent                 = swapChain->getExtent();
                fbDesc.layers                 = 1;
                framebuffers.push_back(device.createFramebuffer(fbDesc));
            }

            std::printf("Swapchain and framebuffers recreated: %ux%u\n",
                        swapChain->getExtent().width, swapChain->getExtent().height);
        };

        // --- Uniform-буфер: одна mat4 ---
        ASH::BufferDesc uboDesc{};
        uboDesc.size   = sizeof(Mat4);
        uboDesc.usage  = ASH::BufferUsage::UniformBuffer;
        uboDesc.memory = ASH::MemoryUsage::CpuToGpu;

        auto uniformBuffer = device.createBuffer(uboDesc);
        std::printf("Uniform buffer created.\n");

        // --- Текстура: генерируем данные, грузим через staging buffer ---
        constexpr uint32_t kTextureSize = 16;
        std::vector<uint8_t> pixels = generateCheckerboardPixels(kTextureSize, kTextureSize);

        ASH::TextureDesc textureDesc{};
        textureDesc.type        = ASH::TextureType::Texture2D;
        textureDesc.format      = ASH::Format::R8G8B8A8_UNorm;
        textureDesc.extent      = { kTextureSize, kTextureSize, 1 };
        textureDesc.mipLevels   = 1;
        textureDesc.arrayLayers = 1;
        textureDesc.usage       = ASH::TextureUsage::Sampled | ASH::TextureUsage::TransferDst;

        auto texture = device.createTexture(textureDesc);
        std::printf("Texture created: %ux%u\n", kTextureSize, kTextureSize);

        ASH::BufferDesc stagingDesc{};
        stagingDesc.size   = pixels.size();
        stagingDesc.usage  = ASH::BufferUsage::TransferSrc;
        stagingDesc.memory = ASH::MemoryUsage::CpuToGpu;

        auto stagingBuffer = device.createBuffer(stagingDesc);

        void* stagingData = stagingBuffer->map();
        std::memcpy(stagingData, pixels.data(), pixels.size());
        stagingBuffer->unmap();

        auto uploadCommandBuffer = device.createCommandBuffer();
        uploadCommandBuffer->begin();

        ASH::TextureBarrier toTransferDst{};
        toTransferDst.texture  = texture.get();
        toTransferDst.oldState = ASH::ResourceState::Undefined;
        toTransferDst.newState = ASH::ResourceState::TransferDst;
        uploadCommandBuffer->barrier(&toTransferDst, 1, nullptr, 0);

        uploadCommandBuffer->copyBufferToTexture(stagingBuffer.get(), texture.get());

        ASH::TextureBarrier toShaderReadOnly{};
        toShaderReadOnly.texture  = texture.get();
        toShaderReadOnly.oldState = ASH::ResourceState::TransferDst;
        toShaderReadOnly.newState = ASH::ResourceState::ShaderReadOnly;
        uploadCommandBuffer->barrier(&toShaderReadOnly, 1, nullptr, 0);

        uploadCommandBuffer->end();

        auto* vulkanUploadCmd = static_cast<ASH::vulkan::VulkanCommandBuffer*>(uploadCommandBuffer.get());
        VkCommandBuffer uploadHandle = vulkanUploadCmd->getHandle();

        VkSubmitInfo uploadSubmitInfo{};
        uploadSubmitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        uploadSubmitInfo.commandBufferCount = 1;
        uploadSubmitInfo.pCommandBuffers    = &uploadHandle;

        vkQueueSubmit(device.getGraphicsQueue(), 1, &uploadSubmitInfo, VK_NULL_HANDLE);
        device.waitIdle();

        std::printf("Texture uploaded to GPU.\n");

        // --- Sampler ---
        ASH::SamplerDesc samplerDesc{};
        samplerDesc.magFilter    = ASH::Filter::Nearest;
        samplerDesc.minFilter    = ASH::Filter::Nearest;
        samplerDesc.addressModeU = ASH::AddressMode::Repeat;
        samplerDesc.addressModeV = ASH::AddressMode::Repeat;

        auto sampler = device.createSampler(samplerDesc);
        std::printf("Sampler created.\n");

        // --- Descriptor Set Layout: два слота — UBO (binding 0) и текстура (binding 1) ---
        ASH::DescriptorBindingDesc uboBinding{};
        uboBinding.binding    = 0;
        uboBinding.type       = ASH::DescriptorType::UniformBuffer;
        uboBinding.count      = 1;
        uboBinding.stageFlags = ASH::ShaderStage::Vertex;

        ASH::DescriptorBindingDesc textureBinding{};
        textureBinding.binding    = 1;
        textureBinding.type       = ASH::DescriptorType::CombinedImageSampler;
        textureBinding.count      = 1;
        textureBinding.stageFlags = ASH::ShaderStage::Fragment;

        ASH::DescriptorSetLayoutDesc layoutDesc{};
        layoutDesc.bindings = { uboBinding, textureBinding };

        auto descriptorSetLayout = device.createDescriptorSetLayout(layoutDesc);
        std::printf("DescriptorSetLayout created.\n");

        // --- Descriptor Set: заполняем ОБА слота ---
        auto descriptorSet = device.createDescriptorSet(descriptorSetLayout.get());

        ASH::DescriptorBufferInfo uboBufferInfo{};
        uboBufferInfo.buffer = uniformBuffer.get();
        uboBufferInfo.offset = 0;
        uboBufferInfo.range  = 0;

        ASH::DescriptorImageInfo textureImageInfo{};
        textureImageInfo.texture = texture.get();
        textureImageInfo.sampler = sampler.get();

        ASH::DescriptorWrite writes[2]{};
        writes[0].binding    = 0;
        writes[0].type       = ASH::DescriptorType::UniformBuffer;
        writes[0].bufferInfo = &uboBufferInfo;

        writes[1].binding   = 1;
        writes[1].type      = ASH::DescriptorType::CombinedImageSampler;
        writes[1].imageInfo = &textureImageInfo;

        descriptorSet->update(writes, 2);
        std::printf("DescriptorSet created and updated.\n");

        // --- Pipeline ---
        std::vector<uint32_t> vertCode = readSpirvFile(std::string(SHADER_DIR) + "triangle.vert.spv");
        std::vector<uint32_t> fragCode = readSpirvFile(std::string(SHADER_DIR) + "triangle.frag.spv");

        ASH::ShaderStageDesc vertStage{};
        vertStage.stage      = ASH::ShaderStage::Vertex;
        vertStage.spirvCode  = vertCode.data();
        vertStage.spirvSize  = vertCode.size() * sizeof(uint32_t);
        vertStage.entryPoint = "main";

        ASH::ShaderStageDesc fragStage{};
        fragStage.stage      = ASH::ShaderStage::Fragment;
        fragStage.spirvCode  = fragCode.data();
        fragStage.spirvSize  = fragCode.size() * sizeof(uint32_t);
        fragStage.entryPoint = "main";

        ASH::GraphicsPipelineDesc pipelineDesc{};
        pipelineDesc.stages = { vertStage, fragStage };
        pipelineDesc.topology = ASH::PrimitiveTopology::TriangleList;
        pipelineDesc.rasterization.polygonMode = ASH::PolygonMode::Fill;
        pipelineDesc.rasterization.cullMode  = ASH::CullMode::None;
        pipelineDesc.rasterization.frontFace = ASH::FrontFace::CounterClockwise;
        pipelineDesc.rasterization.lineWidth = 1.0f;
        pipelineDesc.depthStencil.depthTestEnable  = false;
        pipelineDesc.depthStencil.depthWriteEnable = false;

        ASH::ColorBlendAttachmentState blendAttachment{};
        blendAttachment.blendEnable = false;
        pipelineDesc.colorBlend.attachments = { blendAttachment };

        pipelineDesc.renderPass = renderPass.get();
        pipelineDesc.descriptorSetLayouts = { descriptorSetLayout.get() };

        auto pipeline = device.createGraphicsPipeline(pipelineDesc);
        std::printf("Pipeline created.\n");

        // --- Командные буферы: по одному на каждый frame-in-flight слот ---
        std::vector<std::unique_ptr<ASH::CommandBuffer>> commandBuffers;
        std::vector<ASH::vulkan::VulkanCommandBuffer*>   vulkanCommandBuffers;
        for (uint32_t i = 0; i < ASH::vulkan::VulkanSwapChain::kMaxFramesInFlight; ++i) {
            commandBuffers.push_back(device.createCommandBuffer());
            vulkanCommandBuffers.push_back(static_cast<ASH::vulkan::VulkanCommandBuffer*>(commandBuffers.back().get()));
        }

        uint32_t currentFrame = 0;

        std::printf("Entering render loop. Press ESC or close the window to exit.\n");

        // --- Рендер-цикл ---
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }

            vulkanSwapChain->waitForFrame(currentFrame);

            uint32_t imageIndex = 0;
            if (!vulkanSwapChain->acquireNextImage(imageIndex, currentFrame)) {
                recreateSwapchainResources();
                continue;
            }

            ASH::CommandBuffer* commandBuffer = commandBuffers[currentFrame].get();
            auto* vulkanCommandBuffer = vulkanCommandBuffers[currentFrame];

            commandBuffer->begin();

            ASH::ClearColor clearColor{};
            clearColor.f32[0] = 0.02f;
            clearColor.f32[1] = 0.02f;
            clearColor.f32[2] = 0.05f;
            clearColor.f32[3] = 1.0f;

            ASH::RenderPassBeginInfo rpBegin{};
            rpBegin.renderPass  = renderPass.get();
            rpBegin.framebuffer = framebuffers[imageIndex].get();
            rpBegin.renderArea  = { 0, 0, swapChain->getExtent().width, swapChain->getExtent().height };
            rpBegin.colorClearValues = &clearColor;
            rpBegin.colorClearCount  = 1;
            rpBegin.depthStencilClear = nullptr;

            commandBuffer->beginRenderPass(rpBegin);
            commandBuffer->bindPipeline(pipeline.get());

            float angle = static_cast<float>(glfwGetTime());
            Mat4 rotation = rotationZ(angle);

            void* uboData = uniformBuffer->map();
            std::memcpy(uboData, &rotation, sizeof(Mat4));
            uniformBuffer->unmap();

            commandBuffer->bindDescriptorSet(pipeline.get(), 0, descriptorSet.get());

            ASH::Viewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width    = static_cast<float>(swapChain->getExtent().width);
            viewport.height   = static_cast<float>(swapChain->getExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            commandBuffer->setViewport(viewport);

            ASH::Rect2D scissor{};
            scissor.x = 0;
            scissor.y = 0;
            scissor.width  = swapChain->getExtent().width;
            scissor.height = swapChain->getExtent().height;
            commandBuffer->setScissor(scissor);

            commandBuffer->draw(3, 1, 0, 0);
            commandBuffer->endRenderPass();

            ASH::TextureBarrier presentBarrier{};
            presentBarrier.texture  = swapChain->getImage(imageIndex);
            presentBarrier.oldState = ASH::ResourceState::ColorAttachment;
            presentBarrier.newState = ASH::ResourceState::Present;
            commandBuffer->barrier(&presentBarrier, 1, nullptr, 0);

            commandBuffer->end();

            VkCommandBuffer cmdHandle = vulkanCommandBuffer->getHandle();
            VkSemaphore waitSemaphore   = vulkanSwapChain->getImageAvailableSemaphore(currentFrame);
            VkSemaphore signalSemaphore = vulkanSwapChain->getRenderFinishedSemaphore(imageIndex);
            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            VkSubmitInfo submitInfo{};
            submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount   = 1;
            submitInfo.pWaitSemaphores      = &waitSemaphore;
            submitInfo.pWaitDstStageMask    = &waitStage;
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = &cmdHandle;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores    = &signalSemaphore;

            vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, vulkanSwapChain->getInFlightFence(currentFrame));

            vulkanSwapChain->present(imageIndex, currentFrame);

            currentFrame = (currentFrame + 1) % ASH::vulkan::VulkanSwapChain::kMaxFramesInFlight;
        }

        device.waitIdle();

        std::printf("Done.\n");
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Error: %s\n", e.what());
        exitCode = 1;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return exitCode;
}