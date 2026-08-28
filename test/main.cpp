#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanSwapChain.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstring>
#include <exception>
#include <vector>

int main() {
    if (!glfwInit()) {
        std::fprintf(stderr, "glfwInit failed\n");
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "ASH Test", nullptr, nullptr);
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

        ASH::SwapChainDesc swapChainDesc{};
        swapChainDesc.extent      = { 800, 600 };
        swapChainDesc.format      = ASH::Format::B8G8R8A8_UNorm;
        swapChainDesc.imageCount  = 3;
        swapChainDesc.presentMode = ASH::PresentMode::Fifo;

        auto swapChain = device.createSwapChain(swapChainDesc);
        std::printf("SwapChain created. Image count: %u, extent: %ux%u\n",
                    swapChain->getImageCount(),
                    swapChain->getExtent().width,
                    swapChain->getExtent().height);

        double startTime = glfwGetTime();
        while (!glfwWindowShouldClose(window) && (glfwGetTime() - startTime) < 3.0) {
            glfwPollEvents();
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