#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <set>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <cstring>

#include "fasta_utils_lib/logging.h"
#include "fasta_lib/vulkan_tools.h"
#include "fasta_lib/vulkan_app.h"

Vulkan_App::Vulkan_App() {
    _surface = VK_NULL_HANDLE;
	_physical_device = VK_NULL_HANDLE;
}

void Vulkan_App::initVulkan() {
    LOG_DBG << "Vulkan_App::initVulkan()";
	createInstance();
	setupDebugMessenger();
    createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    LOG_DBG << "Vulkan_App::initVulkan() done";
}

void Vulkan_App::cleanup() {
    LOG_DBG << "Vulkan_App::cleanup()";
    vkDestroyPipeline(_device, _graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    vkDestroyRenderPass(_device, _render_pass, nullptr);

    for (auto image_view : _swap_chain_image_views) {
        vkDestroyImageView(_device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(_device, _swap_chain, nullptr);
	vkDestroyDevice(_device, nullptr);

	if (_enable_validation_layers) {
        DestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
    }

    if ( _surface ) {
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
    }

	vkDestroyInstance(_instance, nullptr);
    LOG_DBG << "Vulkan_App::cleanup() done";
}

void Vulkan_App::createInstance() {
    LOG_DBG << "Vulkan_App::createInstance()";
	if (_enable_validation_layers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (_enable_validation_layers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(_validation_layers.size());
        createInfo.ppEnabledLayerNames = _validation_layers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
    LOG_DBG << "Vulkan_App::createInstance() done";
}

void Vulkan_App::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

std::vector<const char*> Vulkan_App::getRequiredExtensions() {
    LOG_DBG << "Vulkan_App::getRequiredExtensions()";
    LOG_DBG << "Vulkan_App::getRequiredExtensions() done";
    return {};
}

void Vulkan_App::createLogicalDevice() {
    LOG_DBG << "Vulkan_App::createLogicalDevice()";
    QueueFamilyIndices indices = findQueueFamilies(_physical_device);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(_device_extensions.size());
    createInfo.ppEnabledExtensionNames = _device_extensions.data();

    if (_enable_validation_layers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(_validation_layers.size());
        createInfo.ppEnabledLayerNames = _validation_layers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(_physical_device, &createInfo, nullptr, &_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphics_queue);
    vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_present_queue);
    LOG_DBG << "Vulkan_App::createLogicalDevice() done";
}

QueueFamilyIndices Vulkan_App::findQueueFamilies(VkPhysicalDevice device) {
    LOG_DBG << "Vulkan_App::findQueueFamilies()";
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }
    LOG_DBG << "Vulkan_App::findQueueFamilies() done";
    return indices;
}

bool Vulkan_App::isDeviceSuitable(VkPhysicalDevice device) {
    LOG_DBG << "Vulkan_App::isDeviceSuitable()";
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensions_supported = checkDeviceExtensionSupport(device);

    bool swap_chain_adequate = false;
    if (extensions_supported) {
        SwapChainSupportDetails swap_chain_support = querySwapChainSupport(device);
        swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.presentModes.empty();
    }
    LOG_DBG << "Vulkan_App::isDeviceSuitable() done";
    return indices.isComplete() && extensions_supported && swap_chain_adequate;
}

void Vulkan_App::pickPhysicalDevice() {
    LOG_DBG << "Vulkan_App::pickPhysicalDevice()";
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(_instance, &device_count, nullptr);

    if (device_count == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(_instance, &device_count, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            _physical_device = device;
            break;
        }
    }

    if (_physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    LOG_DBG << "Vulkan_App::pickPhysicalDevice() done";
}

SwapChainSupportDetails Vulkan_App::querySwapChainSupport(VkPhysicalDevice device) {
    LOG_DBG << "Vulkan_App::querySwapChainSupport()";
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &format_count, nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.presentModes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &present_mode_count, details.presentModes.data());
    }
    LOG_DBG << "Vulkan_App::querySwapChainSupport() done";
    return details;
}

VkShaderModule Vulkan_App::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(_device, &createInfo, nullptr, &shader_module) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shader_module;
}

bool Vulkan_App::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    LOG_DBG << "Vulkan_App::checkDeviceExtensionSupport()";
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(_device_extensions.begin(), _device_extensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    LOG_DBG << "Vulkan_App::checkDeviceExtensionSupport() done";
    return requiredExtensions.empty();
}

bool Vulkan_App::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : _validation_layers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<char> Vulkan_App::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan_App::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void Vulkan_App::setupDebugMessenger() {
    if (!_enable_validation_layers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debug_messenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

/*
#ifdef NDEBUG
const bool Vulkan_App::_enable_validation_layers = false;
#else
const bool Vulkan_App::_enable_validation_layers = true;
#endif
*/
const bool Vulkan_App::_enable_validation_layers = false;
const std::vector<const char*> Vulkan_App::_validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> Vulkan_App::_device_extensions = {};