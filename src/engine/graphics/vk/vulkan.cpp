//
// Created by Sam Serrels on 30/11/2019.
//

#include "vulkan.h"
#include "../../../game/game.h"
#include "../../platform/platform_glfw.h"
#include "vulkan_internals.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

// info->

std::unique_ptr<ContextInfo> ctx;
std::unique_ptr<SwapChainInfo> swapchain;
vk::UniqueRenderPass renderPass;
std::unique_ptr<Pipeline> pipeline;
std::unique_ptr<CmdPool> cmdPool;
std::unique_ptr<CmdBuffers> cmdBuffers;
std::unique_ptr<SyncObjects> syncObjects;
//
std::unique_ptr<VertexBuffer> vbuffer;
std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
std::unique_ptr<DescriptorPool> descriptorPool;
std::unique_ptr<DescriptorSets> descriptorSets;
//
std::unique_ptr<Uniform> uniform;
//
std::unique_ptr<TextureImage> texture;

void RebuildSwapChain() {
  vkDeviceWaitIdle(ctx->device);
  pipeline.reset();
  renderPass.reset();
  swapchain.reset();

  swapchain = std::make_unique<SwapChainInfo>(ctx->deviceKHR, ctx->device);
  renderPass = createRenderPass(ctx->device, swapchain->swapChainImageFormat);
  swapchain->InitFramebuffers(*renderPass);

  pipeline = std::make_unique<Pipeline>(ctx->device, swapchain->swapChainExtent, *renderPass, Vertex::getPipelineInputState(),
                                        descriptorSetLayout->descriptorSetLayout);

  uniform = std::make_unique<Uniform>(swapchain->swapChainFramebuffers.size(), ctx->device, ctx->physicalDevice);
  descriptorPool = std::make_unique<DescriptorPool>(ctx->device, swapchain->swapChainImages);
  descriptorSets = std::make_unique<DescriptorSets>(ctx->device, swapchain->swapChainImages, descriptorSetLayout->descriptorSetLayout,
                                                    descriptorPool->descriptorPool, uniform->uniformBuffers);
  std::cout << "swapchain Built" << std::endl;
}

void VulkanBackend::startup() {
  ctx = std::make_unique<ContextInfo>();
  cmdPool = std::make_unique<CmdPool>(ctx->deviceKHR, ctx->device);
  descriptorSetLayout = std::make_unique<DescriptorSetLayout>(ctx->device);
  RebuildSwapChain();

  // Unless the amount of swapchain images changes, don't need to rebuild this when swapchain does.
  syncObjects = std::make_unique<SyncObjects>(ctx->device, swapchain->swapChainImages.size());
  cmdBuffers = std::make_unique<CmdBuffers>(ctx->device, cmdPool->commandPool, swapchain->swapChainFramebuffers.size());

  // data

  size_t vc, ic;
  Game::Vertex* vd = Game::getVertices(vc);
  glm::uint16_t* id = Game::getIndices(ic);

  std::vector<Vertex> convertedVertexes;
  for (int i = 0; i < vc; ++i) {
    convertedVertexes.push_back(vd[i]);
  }
  // auto ofsetIndices = std::transform(myIndices.begin(), myIndices.end(), std::back_inserter(ConvertedVertexes), [&i](auto& c){return c+(i*6);});
  const auto vertices_size = sizeof(convertedVertexes[0]) * vc;
  const auto indices_size = sizeof(id[0]) * ic;

  vbuffer = std::make_unique<VertexBuffer>(ctx->device, ctx->physicalDevice, vertices_size, indices_size);
  vbuffer->UploadVertex(&convertedVertexes[0], vertices_size, cmdPool->commandPool, ctx->graphicsQueue);
  vbuffer->UploadIndex(id, indices_size, cmdPool->commandPool, ctx->graphicsQueue);
  // Textures
  texture = std::make_unique<TextureImage>(ctx->device, ctx->physicalDevice, cmdPool->commandPool, ctx->graphicsQueue);

  std::cout << "VK init Done" << std::endl;
}

void VulkanBackend::shutdown() {
  vkDeviceWaitIdle(ctx->device);
  descriptorPool.reset();
  syncObjects.reset();
  cmdBuffers.reset();
  uniform.reset();
  pipeline.reset();
  renderPass.reset();
  swapchain.reset();

  texture.reset();
  vbuffer.reset();
  cmdPool.reset();
  descriptorSetLayout.reset();

  ctx.reset();
}

void VulkanBackend::drawFrame(double dt) {
  // render commands
  size_t ic;
  Game::getIndices(ic);
  static double icc = 0;
  icc += (dt * 30);
  if (icc > ic) {
    icc = 0;
  }
  size_t ics = (size_t)floor(icc);
  ics = ic;
  uint32_t a = WaitForAvilibleImage(ctx->device, swapchain->swapChain, *syncObjects);
  if (a == -1) {
    RebuildSwapChain();
    a = WaitForAvilibleImage(ctx->device, swapchain->swapChain, *syncObjects);
    if (a == -1) {
      throw std::runtime_error("Can't make valid swapChain!");
    }
  }

  cmdBuffers->Record(ctx->device, *renderPass, swapchain->swapChainExtent, swapchain->swapChainFramebuffers, *pipeline, *vbuffer, ics,
                     *descriptorSets, a);

  uniform->updateUniformBuffer(a, dt, swapchain->swapChainExtent);
  drawFrameInternal(a, ctx->device, ctx->graphicsQueue, ctx->presentQueue, swapchain->swapChain, cmdBuffers->commandBuffers, *syncObjects);
}

void VulkanBackend::resize() {
  // need to recreate framebuffers
  //	vkDeviceWaitIdle(vkinfo->device);
  std::cout << "VK recreate" << std::endl;
}
