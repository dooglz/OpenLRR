//
// Created by Sam Serrels on 30/11/2019.
//
#include "vulkan_bootstrap.h"
#include <iostream>
#include <vulkan/vulkan.h>
#include "../../platform/platform_glfw.h"
#include <vector>
#include <optional>
#include <string> 
#include <set>
#include "vulkan_bootstrap.h"


//VkInfo* info;

const bool enableValidationLayers = true;

const std::vector<const char*> validationLayers = {
"VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

static std::string BytesToString(unsigned long long byteCount)
{
	std::string suf[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB" }; //Longs run out around EB
	if (byteCount == 0)
		return "0" + suf[0];
	int place = (floor(log(byteCount)/log(1024)));
	double a = (byteCount / pow(1024, place));
	const int precisionVal = 3;
	const std::string trimmedString = std::to_string(a).substr(0, std::to_string(a).find(".") + precisionVal + 1);
	return trimmedString + suf[place];
}

void probe() {
	VkInstance pinstance;
	VkResult result;
	VkInstanceCreateInfo info = {};
	uint32_t instance_layer_count;

	result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
	std::cout << instance_layer_count << " layers found!\n";
	if (instance_layer_count > 0) {
		std::unique_ptr<VkLayerProperties[]> instance_layers(new VkLayerProperties[instance_layer_count]);
		result = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.get());
		for (int i = 0; i < instance_layer_count; ++i) {
			std::cout << instance_layers[i].layerName << "\n";
		}
	}

}

SwapChainSupportDetails querySwapChainSupport(VkInfo& info) {
	VkPhysicalDevice device = info.physicalDevice;
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, info.surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, info.surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, info.surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, info.surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, info.surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


bool checkValidationLayerSupport() {

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}


void SetupInstance(VkInfo& info) {
	
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "OpenLRR";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "OpenLRR";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;
	///
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	///
	std::vector<const char*> extensions;
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = platform::GetRequiredVKInstanceExtensions(&glfwExtensionCount);
		extensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
	}
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	//
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &info.instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
	std::cout << "Enabled Layers:" << std::endl;
	for (int i = 0; i < createInfo.enabledLayerCount; ++i) {
		std::cout << createInfo.ppEnabledLayerNames[i] << "\n";
	}
	std::cout << "Enabled Extensions:" << std::endl;
	for (int i = 0; i < createInfo.enabledExtensionCount; ++i) {
		std::cout << createInfo.ppEnabledExtensionNames[i] << "\n";
	}

	if (!enableValidationLayers) return;

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(info.instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr && func(info.instance, &debugCreateInfo, nullptr, &info.debugMessenger) == VK_SUCCESS) {
		std::cout << "VK debug messenger created" << std::endl;
	}
	else {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkInfo& info) {
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
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, info.surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device, VkInfo& info) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void pickPhysicalDevice(VkInfo& info) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(info.instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(info.instance, &deviceCount, devices.data());

	std::vector<VkPhysicalDevice> compatableDevices;
	for (const auto& device : devices) {
		if (findQueueFamilies(device,info).isComplete() && checkDeviceExtensionSupport(device,info)) {
			info.physicalDevice = device;
			compatableDevices.push_back(device);
		}
	}

	VkDeviceSize biggestRamAmount =0;
	VkPhysicalDevice BestGPU = VK_NULL_HANDLE;

	std::cout << "comaptible GPU devices:" << "\n";

	for (const auto g : compatableDevices) {
		VkPhysicalDeviceProperties dp;
		vkGetPhysicalDeviceProperties(g, &dp);
		VkDeviceSize memsize = 0;
		{
			auto memoryProps = VkPhysicalDeviceMemoryProperties{};
			vkGetPhysicalDeviceMemoryProperties(g, &memoryProps);
			auto heapsPointer = memoryProps.memoryHeaps;
			auto heaps = std::vector<VkMemoryHeap>(heapsPointer, heapsPointer + memoryProps.memoryHeapCount);
			for (const auto& heap : heaps)
			{
				if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				{
					memsize = heap.size;
					if (memsize > biggestRamAmount) {
						biggestRamAmount = memsize;
						BestGPU = g;
					}
					break;
				}
			}
		}
		std::cout << "Addr: "<< g <<" Device " << dp.deviceID << " - " << dp.deviceName << " - " << dp.driverVersion << " vram: "<< BytesToString(memsize) <<"\n";
	}
	info.physicalDevice = BestGPU;
	if (info.physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	std::cout << "Using Device " << BestGPU << std::endl;

}
void createLogicalDevice(VkInfo& info) {
	QueueFamilyIndices indices = findQueueFamilies(info.physicalDevice,info);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

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

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(info.physicalDevice, &createInfo, nullptr, &info.device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(info.device, indices.graphicsFamily.value(), 0, &info.graphicsQueue);
	vkGetDeviceQueue(info.device, indices.presentFamily.value(), 0, &info.presentQueue);
	std::cout << "Logical Device is cool" << std::endl;
}


void createSwapChain(VkInfo& info) {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(info);

	VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
	{
		for (const auto& availableFormat : swapChainSupport.formats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				surfaceFormat = availableFormat;
				break;
			}
		}
		if (surfaceFormat.format == VK_FORMAT_UNDEFINED) { surfaceFormat = swapChainSupport.formats[0]; }
	}

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	{
		for (const auto& availablePresentMode : swapChainSupport.presentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				presentMode = availablePresentMode;
				break;
			}
		}
	}

	VkExtent2D extent;
	{
		const auto cap = swapChainSupport.capabilities;
		if (cap.currentExtent.width != UINT32_MAX) {
			extent = cap.currentExtent;
		}else {
			VkExtent2D actualExtent = {1280,720};
			actualExtent.width = max(cap.minImageExtent.width, min(cap.maxImageExtent.width, actualExtent.width));
			actualExtent.height = max(cap.minImageExtent.height, min(cap.maxImageExtent.height, actualExtent.height));
			extent = actualExtent;
		}
	}

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = info.surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(info.physicalDevice,info);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(info.device, &createInfo, nullptr, &info.swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(info.device, info.swapChain, &imageCount, nullptr);
	info.swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(info.device, info.swapChain, &imageCount, info.swapChainImages.data());

	info.swapChainImageFormat = surfaceFormat.format;
	info.swapChainExtent = extent;
	std::cout << "Swap Chain is cool: " << info.swapChainExtent.height << "x" << info.swapChainExtent.width << std::endl;
}

void createImageViews(VkInfo& info) {
	info.swapChainImageViews.resize(info.swapChainImages.size());

	for (size_t i = 0; i < info.swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = info.swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = info.swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(info.device, &createInfo, nullptr, &info.swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}


std::unique_ptr<VkInfo> vk_Startup()
{
	probe();
	std::unique_ptr<VkInfo> info = std::make_unique<VkInfo>();
	SetupInstance(*info);
	if (!platform::CreateVKWindowSurface(&(info->instance), &info->surface)) {
		throw std::runtime_error("failed to create window surface!");
	}
	pickPhysicalDevice(*info);
	createLogicalDevice(*info);
	createSwapChain(*info);
	createImageViews(*info);
	//

	return std::unique_ptr<VkInfo>();
}

void vk_Shutdown(std::unique_ptr<VkInfo> info) {
	for (auto imageView : info->swapChainImageViews) {
		vkDestroyImageView(info->device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(info->device, info->swapChain, nullptr);
	vkDestroyDevice(info->device, nullptr);

	if (enableValidationLayers) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(info->instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(info->instance, info->debugMessenger, nullptr);
		}
	}
	vkDestroySurfaceKHR(info->instance, info->surface, nullptr);
	vkDestroyInstance(info->instance, nullptr);
	info.reset();
}