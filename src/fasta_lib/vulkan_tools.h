#ifndef FASTA_LIB_VULKAN_TOOLS__H
#define FASTA_LIB_VULKAN_TOOLS__H

#include <optional>
#include <vulkan/vulkan.h>


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

#endif // FASTA_LIB_VULKAN_TOOLS__H