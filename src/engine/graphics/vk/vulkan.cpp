//
// Created by Sam Serrels on 30/11/2019.
//

#include "vulkan.h"
#include "vulkan_bootstrap.h"
#include <iostream>
#include <vulkan/vulkan.h>
#include "../../platform/platform_glfw.h"

//info->

std::unique_ptr<VkInfo> vkinfo;

void VulkanBackend::startup() {
	vkinfo = vk_Startup();
}

void VulkanBackend::shutdown() {
	vk_Shutdown(std::move(vkinfo));
}
