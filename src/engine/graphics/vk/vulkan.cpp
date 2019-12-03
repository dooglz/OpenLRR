//
// Created by Sam Serrels on 30/11/2019.
//

#include "vulkan.h"
#include "vulkan_internals.h"
#include <iostream>
#include <vulkan/vulkan.h>
#include "../../platform/platform_glfw.h"

//info->

std::unique_ptr<VkInfo> vkinfo;
std::unique_ptr<VkPipelineInfo> vkpipeline;
std::unique_ptr <VkRenderPass> renderPass;
std::unique_ptr<VkCmdInfo> commandPools;
std::unique_ptr<VkSyncObjects> syncObjects;

void VulkanBackend::startup() {
	vkinfo = vk_Startup();
	renderPass = vk_createRenderPass(*vkinfo);
	vkpipeline = vk_InitPipeline(*vkinfo,*renderPass);
	createFramebuffers(*vkinfo, *renderPass);
	commandPools = createCommandPool(*vkinfo, *renderPass,*vkpipeline);
	syncObjects = createSyncObjects(*vkinfo);
	std::cout << "VK init Done" << std::endl;
}

void VulkanBackend::shutdown() {
	vk_DestorySyncObjects(std::move(syncObjects), *vkinfo);
	vk_DestoryCommandPool(std::move(commandPools),*vkinfo);
	vk_DestoryFramebuffer(*vkinfo);
	vk_DestoryPipeline(std::move(vkpipeline), *vkinfo);
	vk_DestoryRenderPass(std::move(renderPass), *vkinfo);
	vk_Shutdown(std::move(vkinfo));
}



void VulkanBackend::drawFrame() {
	VKdrawFrame(*vkinfo, *syncObjects, *commandPools);
}

void cleanupSwapChain();

void VulkanBackend::resize()
{
	//need to recreate framebuffers
	vkDeviceWaitIdle(vkinfo->device);
	std::cout << "VK recreate" << std::endl;
}
