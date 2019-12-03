//
// Created by Sam Serrels on 30/11/2019.
//

#include "vulkan.h"
#include "../../platform/platform_glfw.h"
#include "vulkan_internals.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

// info->

std::unique_ptr<ContextInfo> ctx;
std::unique_ptr<SwapChainInfo> swapchain;
vk::UniqueRenderPass renderPass;
std::unique_ptr<Pipeline> pipeline;
std::unique_ptr<CmdPoolBuf> commandPools;
std::unique_ptr<SyncObjects> syncObjects;

void VulkanBackend::startup() {
  ctx = std::make_unique<ContextInfo>();
  swapchain = std::make_unique<SwapChainInfo>(ctx->deviceKHR, ctx->device);
  renderPass = createRenderPass(ctx->device, swapchain->swapChainImageFormat);
  swapchain->InitFramebuffers(*renderPass);

  pipeline = std::make_unique<Pipeline>(ctx->device, swapchain->swapChainExtent, *renderPass);
  commandPools = std::make_unique<CmdPoolBuf>(ctx->deviceKHR, ctx->device, *renderPass, pipeline->graphicsPipeline, swapchain->swapChainFramebuffers,
                                              swapchain->swapChainExtent);
  syncObjects = std::make_unique<SyncObjects>(ctx->device, swapchain->swapChainImages);

  std::cout << "VK init Done" << std::endl;
}

void RebuildSwapChain() {}

void VulkanBackend::shutdown() {
  vkDeviceWaitIdle(ctx->device);
  syncObjects.reset();
  commandPools.reset();
  pipeline.reset();
  renderPass.reset();
  swapchain.reset();
  ctx.reset();
}

void VulkanBackend::drawFrame() {
  drawFrameInternal(ctx->device, ctx->graphicsQueue, ctx->presentQueue, swapchain->swapChain, commandPools->commandBuffers, *syncObjects);
}

void VulkanBackend::resize() {
  // need to recreate framebuffers
  //	vkDeviceWaitIdle(vkinfo->device);
  std::cout << "VK recreate" << std::endl;
}
