#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <cstring>

#include "fasta_utils_lib/logging.h"
#include "fasta_lib/vulkan_tools.h"
#include "ipr_window.h"

IPR_Window::IPR_Window(): Vulkan_App() {
	_width = 800;
	_height = 600;

	_window = nullptr;
    _show_debug_info = false;
}

IPR_Window::IPR_Window(uint width, uint height): IPR_Window() {
	_width = width;
	_height = height;
}

void IPR_Window::run() {
    LOG_DBG << "IPR_Window::run()";
    initWindow();
	initVulkan();

	mainLoop();
	
    cleanup();
    LOG_DBG << "IPR_Window::run() done";
}

 void IPR_Window::mainLoop() {
    LOG_DBG << "IPR_Window::mainLoop()";
    int samples = 128;

    while (!glfwWindowShouldClose(_window)) {
        glfwGetFramebufferSize(_window, &_width, &_height);
        glfwPollEvents();

        drawFrame();

        // Start the Dear ImGui frame
        //ImGui_ImplVulkan_NewFrame();
        //ImGui_ImplGlfw_NewFrame();
        //ImGui::NewFrame();

        //if (_show_debug_info) {
        //    ImGui::Begin("Debug window", &_show_debug_info);
        //    ImGui::Text("Window average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //    ImGui::SliderInt("Samples", &samples, 1, 128);
        //    if (ImGui::Button("Close"))
        //        _show_debug_info = false;
        //    ImGui::End();
        //}

        //ImGui::Render();
        //memcpy(&wd->ClearValue.color.float32[0], &clear_color, 4 * sizeof(float));
        //frameRender(wd);
        //framePresent(wd);
    }
    vkDeviceWaitIdle(_device);

    LOG_DBG << "IPR_Window::mainLoop() done";
}

void IPR_Window::initWindow() {
    LOG_DBG << "IPR_Window::initWindow()";
	glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    _window = glfwCreateWindow(_width, _height, "Fasta IPR (Vulkan)", nullptr, nullptr);

    glfwSetWindowUserPointer(_window, this);
    glfwSetKeyCallback(_window, keyCallback);
    glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
    LOG_DBG << "IPR_Window::initWindow() done";
}

void IPR_Window::createSurface() {
    LOG_DBG << "IPR_Window::createSurface()";
    if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
    LOG_DBG << "IPR_Window::createSurface() done";
}

void IPR_Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    IPR_Window *ipr_window = static_cast<IPR_Window*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS) {
        switch(key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_P:
                //fasta_ipr->togglePauseResume();
                break;
            case GLFW_KEY_S:
                //fasta_ipr->toggleStartStop();
                break;
            case GLFW_KEY_D:
                ipr_window->toggleShowDebugInfo();
                break;
        }
    }
}

void IPR_Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto ipr_window = reinterpret_cast<IPR_Window*>(glfwGetWindowUserPointer(window));
    ipr_window->framebufferResized = true;
}

void IPR_Window::toggleShowDebugInfo() {
    _show_debug_info = !_show_debug_info;
}

void  IPR_Window::cleanupSwapChain() {
    for (auto framebuffer : _swap_chain_framebuffers) {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(_device, _command_pool, static_cast<uint32_t>(_command_buffers.size()), _command_buffers.data());

    vkDestroyPipeline(_device, _graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    vkDestroyRenderPass(_device, _render_pass, nullptr);

    for (auto image_view : _swap_chain_image_views) {
        vkDestroyImageView(_device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(_device, _swap_chain, nullptr);
}

void IPR_Window::cleanup() {
    LOG_DBG << "IPR_Window::cleanup()";
    //ImGui_ImplVulkan_Shutdown();
    //ImGui_ImplGlfw_Shutdown();
    //ImGui::DestroyContext();

    cleanupSwapChain();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(_device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(_device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(_device, inFlightFences[i], nullptr);
    }

	Vulkan_App::cleanup();
	
    glfwDestroyWindow(_window);
    glfwTerminate();
    LOG_DBG << "IPR_Window::cleanup() done";
}

void IPR_Window::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(_window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(_device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandBuffers();
}

void IPR_Window::createSwapChain() {
    LOG_DBG << "IPR_Window::createSwapChain()";
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physical_device);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t image_count = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && image_count > swapChainSupport.capabilities.maxImageCount) {
        image_count = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = _surface;

    create_info.minImageCount = image_count;
    create_info.imageFormat = surfaceFormat.format;
    create_info.imageColorSpace = surfaceFormat.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(_physical_device);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = swapChainSupport.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = presentMode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_device, &create_info, nullptr, &_swap_chain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(_device, _swap_chain, &image_count, nullptr);
    _swap_chain_images.resize(image_count);
    vkGetSwapchainImagesKHR(_device, _swap_chain, &image_count, _swap_chain_images.data());

    _swap_chain_image_format = surfaceFormat.format;
    _swap_chain_extent = extent;

    LOG_DBG << "IPR_Window::createSwapChain() done";
}

void IPR_Window::createImageViews() {
    LOG_DBG << "IPR_Window::createImageViews()";
    _swap_chain_image_views.resize(_swap_chain_images.size());

    for (size_t i = 0; i < _swap_chain_images.size(); i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = _swap_chain_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = _swap_chain_image_format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(_device, &createInfo, nullptr, &_swap_chain_image_views[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
    LOG_DBG << "IPR_Window::createImageViews() done";
}

void IPR_Window::createRenderPass() {
    LOG_DBG << "IPR_Window::createRenderPass()";
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = _swap_chain_image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
    LOG_DBG << "IPR_Window::createRenderPass() done";
}

void IPR_Window::createGraphicsPipeline() {
    LOG_DBG << "IPR_Window::createGraphicsPipeline()";
    auto vertShaderCode = readFile("../shaders/vert.spv");
    auto fragShaderCode = readFile("../shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) _swap_chain_extent.width;
    viewport.height = (float) _swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = _swap_chain_extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = _pipeline_layout;
    pipelineInfo.renderPass = _render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphics_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(_device, vertShaderModule, nullptr);
    LOG_DBG << "IPR_Window::createGraphicsPipeline() done";
}

void IPR_Window::createFramebuffers() {
    LOG_DBG << "IPR_Window::createFramebuffers()";
    _swap_chain_framebuffers.resize(_swap_chain_image_views.size());

    for (size_t i = 0; i < _swap_chain_image_views.size(); i++) {
        VkImageView attachments[] = {
            _swap_chain_image_views[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = _swap_chain_extent.width;
        framebufferInfo.height = _swap_chain_extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swap_chain_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
    LOG_DBG << "IPR_Window::createFramebuffers() done";
}

VkSurfaceFormatKHR IPR_Window::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR IPR_Window::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D IPR_Window::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {_width, _height};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

std::vector<const char*> IPR_Window::getRequiredExtensions() {
    LOG_DBG << "IPR_Window::getRequiredExtensions()";
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (_enable_validation_layers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    LOG_DBG << "IPR_Window::getRequiredExtensions() done";
    return extensions;
}

void IPR_Window::createCommandBuffers() {
    LOG_DBG << "IPR_Window::createCommandBuffers()";
    _command_buffers.resize(_swap_chain_framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) _command_buffers.size();

    if (vkAllocateCommandBuffers(_device, &allocInfo, _command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < _command_buffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(_command_buffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _render_pass;
        renderPassInfo.framebuffer = _swap_chain_framebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _swap_chain_extent;

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(_command_buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);

            vkCmdDraw(_command_buffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(_command_buffers[i]);

        if (vkEndCommandBuffer(_command_buffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
    LOG_DBG << "IPR_Window::createCommandBuffers() done";
}

void IPR_Window::createSyncObjects() {
    LOG_DBG << "IPR_Window::createSyncObjects()";
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(_swap_chain_images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(_device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
    LOG_DBG << "IPR_Window::createSyncObjects() done";
}

void IPR_Window::drawFrame() {
    vkWaitForFences(_device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(_device, _swap_chain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(_device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_command_buffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(_device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(_graphics_queue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {_swap_chain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(_present_queue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

const std::vector<const char*> Vulkan_App::_device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};