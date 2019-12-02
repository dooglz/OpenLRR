//
// Created by Sam Serrels on 30/11/2019.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

struct VkInfo
{
	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device; //AKA logical device

	VkSurfaceKHR surface;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	VkDebugUtilsMessengerEXT debugMessenger;
};

std::unique_ptr<VkInfo> vk_Startup();
void vk_Shutdown(std::unique_ptr<VkInfo>);