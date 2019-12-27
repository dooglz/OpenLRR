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
//std::unique_ptr<VertexBuffer> vbuffer;
std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
std::unique_ptr<DescriptorPool> descriptorPool;
std::unique_ptr<DescriptorSets> descriptorSets;
//
std::unique_ptr<Uniform> uniform;
//
std::unique_ptr<TextureImage> texture;
//
std::vector<vkRenderableItem*> totalRIs;
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
  descriptorSets =
      std::make_unique<DescriptorSets>(ctx->device, swapchain->swapChainImages, descriptorSetLayout->descriptorSetLayout,
                                       descriptorPool->descriptorPool, uniform->uniformBuffers, texture->_imageView, texture->_imageSampler);
  std::cout << "swapchain Built" << std::endl;
}

void VulkanBackend::startup() {
  ctx = std::make_unique<ContextInfo>();
  cmdPool = std::make_unique<CmdPool>(ctx->deviceKHR, ctx->device);
  descriptorSetLayout = std::make_unique<DescriptorSetLayout>(ctx->device);

  // Textures
  texture = std::make_unique<TextureImage>(ctx->device, ctx->physicalDevice, cmdPool->commandPool, ctx->graphicsQueue);

  RebuildSwapChain();

  // Unless the amount of swapchain images changes, don't need to rebuild this when swapchain does.
  syncObjects = std::make_unique<SyncObjects>(ctx->device, swapchain->swapChainImages.size());
  cmdBuffers = std::make_unique<CmdBuffers>(ctx->device, cmdPool->commandPool, swapchain->swapChainFramebuffers.size());
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
//  vbuffer.reset();
  cmdPool.reset();
  descriptorSetLayout.reset();

  ctx.reset();
}

void VulkanBackend::drawFrame(double dt) {
  // render commands

  uint32_t a = WaitForAvilibleImage(ctx->device, swapchain->swapChain, *syncObjects);
  if (a == -1) {
    RebuildSwapChain();
    a = WaitForAvilibleImage(ctx->device, swapchain->swapChain, *syncObjects);
    if (a == -1) {
      throw std::runtime_error("Can't make valid swapChain!");
    }
  }

  for (int i = 0; i < totalRIs.size(); ++i) {
     const vkRenderableItem& ri = *totalRIs[i];
    cmdBuffers->Record(ctx->device, *renderPass, swapchain->swapChainExtent, swapchain->swapChainFramebuffers, *pipeline, *ri._vbuffer, ri._icount,
                       *descriptorSets, a);
  }


  uniform->updateUniformBuffer(a, dt, swapchain->swapChainExtent);
  drawFrameInternal(a, ctx->device, ctx->graphicsQueue, ctx->presentQueue, swapchain->swapChain, cmdBuffers->commandBuffers, *syncObjects);
}

void VulkanBackend::resize() {
  // need to recreate framebuffers
  //	vkDeviceWaitIdle(vkinfo->device);
  std::cout << "VK recreate" << std::endl;
}


vkRenderableItem::~vkRenderableItem() {
  _vbuffer.reset();
}
void vkRenderableItem::updateData(Game::Vertex* vertices, size_t vcount, glm::uint16_t* indices, size_t icount) {

}

vkRenderableItem::vkRenderableItem(Game::Vertex* vertices, size_t vcount, glm::uint16_t* indices, size_t icount, RenderableItem::PIPELINE p)
    : RenderableItem(vertices, vcount, indices, icount, p) {
  totalRIs.push_back(this);

  std::vector<Vertex> convertedVertexes;
  size_t barrychuckle = 0;

  for (int j = 0; j < _vcount; ++j) {
    convertedVertexes.push_back(vertices[j]);
  }

  for (int j = 0; j < _icount; ++j) {
    switch (barrychuckle % 6) {
    case 2:
      convertedVertexes[indices[j]].barry = glm::vec3(1, 0, 0);
      break;
    case 1:
      convertedVertexes[indices[j]].barry = glm::vec3(0, 1, 0);
      break;
    case 0:
      convertedVertexes[indices[j]].barry = glm::vec3(0, 0, 1);
      break;
    case 4:
      convertedVertexes[indices[j]].barry = glm::vec3(1, 0, 0);
      break;
    case 5:
      convertedVertexes[indices[j]].barry = glm::vec3(0, 1, 0);
      break;
    case 3:
      convertedVertexes[indices[j]].barry = glm::vec3(0, 0, 1);
      break;
    }
    barrychuckle++;
  }
  const auto vertices_size = sizeof(convertedVertexes[0]) * _vcount;
  const auto indices_size = sizeof(indices[0]) * _icount;
  _vbuffer = std::make_unique<VertexBuffer>(ctx->device, ctx->physicalDevice, vertices_size, indices_size);
  _vbuffer->UploadVertex(&convertedVertexes[0], vertices_size, cmdPool->commandPool, ctx->graphicsQueue);
  _vbuffer->UploadIndex(indices, indices_size, cmdPool->commandPool, ctx->graphicsQueue);
}
