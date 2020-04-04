#ifndef FASTA_IPR_WINDOW__H
#define FASTA_IPR_WINDOW__H

#include <vector>
#include <vulkan/vulkan.h>

#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_glfw.h"
#include "imgui/examples/imgui_impl_vulkan.h"

#include "fasta_lib/vulkan_app.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

class IPR_Window: public Vulkan_App {
	public:
		IPR_Window();
		IPR_Window(uint width, uint height);
		//~IPR_Window(){};
		
		void run();

		void toggleShowDebugInfo();

	private:
		void drawFrame();
		void initWindow();
		void mainLoop();
		void cleanup();
		void createSurface();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createCommandBuffers();
        void createSyncObjects();
        void cleanupSwapChain();
        void recreateSwapChain();
		std::vector<const char*> getRequiredExtensions();

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	private:
		int _width, _height;
		GLFWwindow* _window;
		bool _show_debug_info;

	private:
	    std::vector<VkImage> _swap_chain_images;
	    VkFormat _swap_chain_image_format;
	    VkExtent2D _swap_chain_extent;

	    std::vector<VkSemaphore> imageAvailableSemaphores;
    	std::vector<VkSemaphore> renderFinishedSemaphores;
    	std::vector<VkFence> inFlightFences;
    	std::vector<VkFence> imagesInFlight;

    	size_t currentFrame = 0;
    	bool framebufferResized = false;

	private:
		static const std::vector<const char*> _device_extensions;

	// GLFW part
	private:
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};


#endif // FASTA_IPR_WINDOW__H