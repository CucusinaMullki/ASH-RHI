#include "ASH/ASH.h"
#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanCommandBuffer.h"
#include "vulkan/VulkanSwapChain.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>

namespace
{

std::vector<uint32_t> readSpirvFile(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));

    return buffer;
}

std::vector<uint8_t> generateCheckerboardPixels(uint32_t width, uint32_t height)
{
    std::vector<uint8_t> pixels(width * height * 4);

    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
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

}

int main()
{
    if (!glfwInit())
    {
        std::printf("Failed to initialize glfw");
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Render", nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        return 1;
    }

    {
    uint32_t extCount = 0;
    const char** exts = glfwGetRequiredInstanceExtensions(&extCount);
    std::vector<const char*> requiredExtensions(exts, exts + extCount);

    ASH::vulkan::VulkanDevice device(true, requiredExtensions);
    std::printf("Device created.\n");

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult surfaceResult = glfwCreateWindowSurface(device.getInstance(), window, nullptr, &surface);
    if (surfaceResult != VK_SUCCESS)
    {
        std::fprintf(stderr, "glfwCreateWindowSurface failed: %d \n", surfaceResult);
        return 1;
    }
    device.attachSurface(surface);

    ASH::SwapChainDesc swapChainDesc{};
    swapChainDesc.extent = {800, 600};
    swapChainDesc.format = ASH::Format::B8G8R8A8_UNorm;
    swapChainDesc.imageCount = 3;
    swapChainDesc.presentMode = ASH::PresentMode::Fifo;

    auto swapChain = device.createSwapChain(swapChainDesc);

    auto* vulkanSwapChain = static_cast<ASH::vulkan::VulkanSwapChain*>(swapChain.get());

    ASH::AttachmentDesc colorAttachment{};
    colorAttachment.format = ASH::Format::B8G8R8A8_UNorm;
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
    
    ASH::DescriptorBindingDesc textureBinding{};
    textureBinding.binding = 0;
    textureBinding.type = ASH::DescriptorType::CombinedImageSampler;
    textureBinding.stageFlags = ASH::ShaderStage::Fragment;

    ASH::DescriptorSetLayoutDesc layoutDesc{};
    layoutDesc.bindings = { textureBinding };

    auto descriptorSetLayout = device.createDescriptorSetLayout(layoutDesc);

    auto descriptorSet = device.createDescriptorSet(descriptorSetLayout.get());
    
    constexpr uint32_t kTextureSize = 16;
    std::vector<uint8_t> pixels = generateCheckerboardPixels(kTextureSize, kTextureSize);

    ASH::TextureDesc textureDesc{};
    textureDesc.type = ASH::TextureType::Texture2D;
    textureDesc.format = ASH::Format::R8G8B8A8_UNorm;
    textureDesc.extent = { kTextureSize, kTextureSize, 1 };
    textureDesc.mipLevels = 1;
    textureDesc.arrayLayers = 1;
    textureDesc.usage = ASH::TextureUsage::Sampled | ASH::TextureUsage::TransferDst;

    auto texture = device.createTexture(textureDesc);
    std::printf("Texture created.\n");

    ASH::BufferDesc stagingDesc{};
    stagingDesc.size = pixels.size();
    stagingDesc.usage = ASH::BufferUsage::TransferSrc;
    stagingDesc.memory = ASH::MemoryUsage::CpuToGpu;
    auto stagingBuffer = device.createBuffer(stagingDesc);

    void* stagingData = stagingBuffer->map();
    std::memcpy(stagingData, pixels.data(), pixels.size());
    stagingBuffer->unmap();

    auto uploadCmd = device.createCommandBuffer();
    uploadCmd->begin();

    ASH::TextureBarrier toDst{};
    toDst.texture = texture.get();
    toDst.oldState = ASH::ResourceState::Undefined;
    toDst.newState = ASH::ResourceState::TransferDst;
    uploadCmd->barrier(&toDst, 1, nullptr, 0);

    uploadCmd->copyBufferToTexture(stagingBuffer.get(), texture.get());

    ASH::TextureBarrier toRead{};
    toRead.texture = texture.get();
    toRead.oldState = ASH::ResourceState::TransferDst;
    toRead.newState = ASH::ResourceState::ShaderReadOnly;
    uploadCmd->barrier(&toRead, 1, nullptr, 0);

    uploadCmd->end();

    auto* vulkanUploadCmd = static_cast<ASH::vulkan::VulkanCommandBuffer*>(uploadCmd.get());
    VkCommandBuffer uploadHandle = vulkanUploadCmd->getHandle();

    ASH::SamplerDesc samplerDesc{};
    samplerDesc.magFilter = ASH::Filter::Nearest;
    samplerDesc.minFilter = ASH::Filter::Nearest;
    samplerDesc.addressModeU = ASH::AddressMode::Repeat;
    samplerDesc.addressModeV = ASH::AddressMode::Repeat;
    auto sampler = device.createSampler(samplerDesc);

    ASH::DescriptorImageInfo textureImageInfo{};
    textureImageInfo.texture = texture.get();
    textureImageInfo.sampler = sampler.get();

    ASH::DescriptorWrite write{};
    write.binding = 0;
    write.type = ASH::DescriptorType::CombinedImageSampler;
    write.imageInfo = &textureImageInfo;

    descriptorSet->update(&write, 1);

    VkSubmitInfo uploadSubmit{};
    uploadSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    uploadSubmit.commandBufferCount = 1;
    uploadSubmit.pCommandBuffers = &uploadHandle;
    vkQueueSubmit(device.getGraphicsQueue(), 1, &uploadSubmit, VK_NULL_HANDLE);
    device.waitIdle();

    std::vector<uint32_t> vertCode = readSpirvFile(std::string(SHADER_DIR) + "triangle.vert.spv");
    std::vector<uint32_t> fragCode = readSpirvFile(std::string(SHADER_DIR) + "triangle.frag.spv");

    ASH::ShaderStageDesc vertStage{};
    vertStage.stage = ASH::ShaderStage::Vertex;
    vertStage.spirvCode = vertCode.data();
    vertStage.spirvSize = vertCode.size() * sizeof(uint32_t);
    vertStage.entryPoint = "main";

    ASH::ShaderStageDesc fragStage{};
    fragStage.stage = ASH::ShaderStage::Fragment;
    fragStage.spirvCode = fragCode.data();
    fragStage.spirvSize = fragCode.size() * sizeof(uint32_t);
    fragStage.entryPoint = "main";

    ASH::GraphicsPipelineDesc pipelineDesc{};
    pipelineDesc.stages = { vertStage, fragStage };
    pipelineDesc.topology = ASH::PrimitiveTopology::TriangleList;
    pipelineDesc.rasterization.cullMode = ASH::CullMode::None;
    pipelineDesc.depthStencil.depthTestEnable = false;
    pipelineDesc.depthStencil.depthWriteEnable = false;
    pipelineDesc.descriptorSetLayouts = { descriptorSetLayout.get() };

    ASH::ColorBlendAttachmentState blend;
    blend.blendEnable = false;
    pipelineDesc.colorBlend.attachments = { blend };

    pipelineDesc.renderPass = renderPass.get();

    auto pipeline = device.createGraphicsPipeline(pipelineDesc);
    std::printf("Pipeline created.\n");

    ASH::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain->getExtent().width);
    viewport.height = static_cast<float>(swapChain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    ASH::Rect2D scissor{};
    scissor.x = 0;
    scissor.y = 0;
    scissor.width = swapChain->getExtent().width;
    scissor.height = swapChain->getExtent().height;


    constexpr uint32_t kMaxFramesInFlight = 2;

    std::vector<std::unique_ptr<ASH::Semaphore>> imageAvailableSemaphores;
    std::vector<std::unique_ptr<ASH::Fence>> inFlightFences;
    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
    {
        imageAvailableSemaphores.push_back(device.createSemaphore());
        inFlightFences.push_back(device.createFence(/*initiallySignaled=*/true));
    }

    std::vector<std::unique_ptr<ASH::Semaphore>> renderFinishedSemaphores;
    for (uint32_t i = 0; i < swapChain->getImageCount(); ++i)
    {
        renderFinishedSemaphores.push_back(device.createSemaphore());
    }

    std::vector<std::unique_ptr<ASH::CommandBuffer>> commandBuffers;
    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
    {
        commandBuffers.push_back(device.createCommandBuffer());
    }
    std::printf("Command buffer created.\n");

    uint32_t currentFrame = 0;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        inFlightFences[currentFrame]->wait();

        uint32_t imageIndex = 0;
        if (!swapChain->acquireNextImage(imageIndex, imageAvailableSemaphores[currentFrame].get()))
        {
            continue;
        }

        inFlightFences[currentFrame]->reset();

        auto* commandBuffer = commandBuffers[currentFrame].get();
        commandBuffer->begin();

        ASH::ClearColor clearColor{};
        clearColor.f32[0] = 0.02f;
        clearColor.f32[1] = 0.02f;
        clearColor.f32[2] = 0.05f;
        clearColor.f32[3] = 1.0f;

        ASH::RenderPassBeginInfo rpBegin{};
        rpBegin.renderPass = renderPass.get();
        rpBegin.framebuffer = framebuffers[imageIndex].get();
        rpBegin.renderArea = { 0, 0, swapChain->getExtent().width, swapChain->getExtent().height };
        rpBegin.colorClearValues = &clearColor;
        rpBegin.colorClearCount = 1;

        commandBuffer->beginRenderPass(rpBegin);
        commandBuffer->bindPipeline(pipeline.get());
        commandBuffer->bindDescriptorSet(pipeline.get(), 0, descriptorSet.get());
        commandBuffer->setViewport(viewport);
        commandBuffer->setScissor(scissor);
        commandBuffer->draw(3, 1, 0, 0);
        commandBuffer->endRenderPass();

        ASH::TextureBarrier presentBarrier{};
        presentBarrier.texture = swapChain->getImage(imageIndex);
        presentBarrier.oldState = ASH::ResourceState::ColorAttachment;
        presentBarrier.newState = ASH::ResourceState::Present;
        commandBuffer->barrier(&presentBarrier, 1, nullptr, 0);

        commandBuffer->end();

        ASH::SubmitInfo submitInfo{};
        submitInfo.commandBuffer = commandBuffer;
        submitInfo.waitSemaphore = imageAvailableSemaphores[currentFrame].get();
        submitInfo.waitStage = ASH::PipelineStage::ColorAttachmentOutput;
        submitInfo.signalSemaphore = renderFinishedSemaphores[imageIndex].get();
        submitInfo.signalFence = inFlightFences[currentFrame].get(); 

        device.submit(submitInfo);

        swapChain->present(imageIndex, renderFinishedSemaphores[imageIndex].get());

        currentFrame = (currentFrame + 1) % kMaxFramesInFlight;
    }

    device.waitIdle();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}