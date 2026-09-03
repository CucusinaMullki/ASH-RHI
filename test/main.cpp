#include "ASH/ASH.h"
#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanCommandBuffer.h"
#include "vulkan/VulkanSwapChain.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    glfwCreateWindowSurface(device.getInstance(), window, nullptr, &surface);
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

    ASH::ColorBlendAttachmentState blend;
    blend.blendEnable = false;
    pipelineDesc.colorBlend.attachments = { blend };

    pipelineDesc.renderPass = renderPass.get();

    auto pipeline = device.createGraphicsPipeline(pipelineDesc);

    auto commandBuffer = device.createCommandBuffer();

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

    uint32_t currentFrame = 0;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        uint32_t imageIndex = 0;
        if (!swapChain->acquireNextImage(imageIndex))
        {
            continue;
        }

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

        auto* vulkanCmd = static_cast<ASH::vulkan::VulkanCommandBuffer*>(commandBuffer.get());
        VkCommandBuffer cmdHandle = vulkanCmd->getHandle();

        VkSemaphore waitSemaphore   = vulkanSwapChain->getImageAvailableSemaphore(0);
        VkSemaphore signalSemaphore = vulkanSwapChain->getRenderFinishedSemaphore(imageIndex);
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &waitSemaphore;
        submitInfo.pWaitDstStageMask = &waitStage;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdHandle;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signalSemaphore;

        vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

        swapChain->present(imageIndex);

        device.waitIdle();
    }

    device.waitIdle();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}