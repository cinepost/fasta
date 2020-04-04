#ifndef FASTA_VULKAN_APP__H
#define FASTA_VULKAN_APP__H

#include <vector>
#include <vulkan/vulkan.h>

#include "fasta_lib/vulkan_tools.h"


struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Vulkan_App {
	public:
		Vulkan_App();
		~Vulkan_App(){};
	

	protected:
		void initVulkan();
		void cleanup();
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		VkShaderModule createShaderModule(const std::vector<char>& code);
		static std::vector<char> readFile(const std::string& filename);

	private:
		virtual void createSurface(){};
		virtual void createSwapChain(){};
		virtual void createImageViews(){};
        virtual void createRenderPass(){};
        virtual void createGraphicsPipeline(){};
		virtual std::vector<const char*> getRequiredExtensions();
		void createInstance();
		void setupDebugMessenger();
		void createLogicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device);
		void pickPhysicalDevice();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		bool checkValidationLayerSupport();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);		

	protected:
		VkInstance _instance;
		VkSurfaceKHR _surface;
		VkDevice _device;
		VkPhysicalDevice _physical_device;

		VkQueue _present_queue;
		VkQueue _graphics_queue;

		VkSwapchainKHR _swap_chain;
		std::vector<VkImageView> _swap_chain_image_views;

		VkRenderPass _render_pass;
    	VkPipelineLayout _pipeline_layout;
    	VkPipeline _graphics_pipeline;

	private:
		VkDebugUtilsMessengerEXT _debug_messenger;

	protected:
		static const bool _enable_validation_layers;
		static const std::vector<const char*> _validation_layers;

	private:
		static const std::vector<const char*> _device_extensions;
};


#endif // FASTA_VULKAN_APP__H