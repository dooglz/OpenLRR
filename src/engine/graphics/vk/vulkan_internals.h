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
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	VkDebugUtilsMessengerEXT debugMessenger;
};

struct VkCmdInfo {
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
};

std::unique_ptr<VkInfo> vk_Startup();
void vk_Shutdown(std::unique_ptr<VkInfo>);

struct VkPipelineInfo {
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
};

std::unique_ptr<VkPipelineInfo> vk_InitPipeline(const VkInfo& info, const VkRenderPass&);
void vk_DestoryPipeline(std::unique_ptr<VkPipelineInfo>, const VkInfo& info);

std::unique_ptr <VkRenderPass> vk_createRenderPass(const VkInfo& info);
void vk_DestoryRenderPass(std::unique_ptr<VkRenderPass>, const VkInfo& info);

void createFramebuffers(VkInfo& info, const VkRenderPass& renderPass);
void vk_DestoryFramebuffer(VkInfo& info);

std::unique_ptr<VkCmdInfo> createCommandPool(const VkInfo& info, const VkRenderPass& renderPass, const VkPipelineInfo& pipeline);
void vk_DestoryCommandPool(std::unique_ptr < VkCmdInfo>,VkInfo& info);


struct VkSyncObjects {
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;
	const int MAX_FRAMES_IN_FLIGHT = 2;
};
std::unique_ptr<VkSyncObjects>  createSyncObjects(VkInfo& info);
void vk_DestorySyncObjects(std::unique_ptr < VkSyncObjects>, VkInfo& info);


void VKdrawFrame(const VkInfo& info, VkSyncObjects& sync, const VkCmdInfo& CommandPools);
