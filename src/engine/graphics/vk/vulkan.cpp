//
// Created by Sam Serrels on 30/11/2019.
//

#include "vulkan.h"
#include "../../../game/game.h"
#include "../../Engine.h"
#include "../../platform/platform_glfw.h"
#include "vulkan_internals.h"
#include <functional>
#include <iostream>
#include <vulkan/vulkan.hpp>

std::unique_ptr<ContextInfo> ctx;
std::unique_ptr<SwapChainInfo> swapchain;
vk::UniqueRenderPass renderPass;
std::unique_ptr<Pipeline> pipelines[RenderableItem::PIPELINE_COUNT];
std::unique_ptr<CmdPool> cmdPool;
std::unique_ptr<CmdBuffers> cmdBuffers;
std::unique_ptr<SyncObjects> syncObjects;
//
std::unique_ptr<DescriptorPool> descriptorPool;
//
std::vector<vkRenderableItem*> totalRIs;
void RebuildSwapChain() {
  vkDeviceWaitIdle(ctx->device);
  for (size_t i = 0; i < RenderableItem::PIPELINE_COUNT; i++) {
    pipelines[i].reset();
  }
  renderPass.reset();
  swapchain.reset();

  swapchain = std::make_unique<SwapChainInfo>(ctx->deviceKHR, ctx->device);
  renderPass = createRenderPass(ctx->device, swapchain->swapChainImageFormat);
  swapchain->InitFramebuffers(*renderPass);

  descriptorPool = std::make_unique<DescriptorPool>(ctx->device, swapchain->swapChainImages);

  // Make pipleines
  for (size_t i = 0; i < RenderableItem::PIPELINE_COUNT; i++) {
    switch (i) {
    case RenderableItem::lit: {
      auto pl = std::make_unique<vLitPipeline>(ctx->device, swapchain->swapChainExtent, *renderPass);
      pl->generatePipelineResources(ctx->physicalDevice, swapchain->swapChainImages, swapchain->swapChainFramebuffers, descriptorPool->descriptorPool,
                                    cmdPool->commandPool, ctx->graphicsQueue);
      pipelines[i] = std::move(pl);
      break;
    }
    default:
      // pipelines[i] = std::make_unique<Pipeline>(ctx->device, swapchain->swapChainExtent, *renderPass, Vertex::getPipelineInputState(),
      // descriptorSetLayout->descriptorSetLayout);
      break;
    }
  }

  std::cout << "swapchain re-Built" << std::endl;
}

void VulkanBackend::startup() {
  ctx = std::make_unique<ContextInfo>();
  cmdPool = std::make_unique<CmdPool>(ctx->deviceKHR, ctx->device);
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
  for (size_t i = 0; i < RenderableItem::PIPELINE_COUNT; i++) {
    pipelines[i].reset();
  }
  renderPass.reset();
  swapchain.reset();
  cmdPool.reset();
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

  for (size_t i = 0; i < RenderableItem::PIPELINE_COUNT; i++) {
    switch (i) {
    case RenderableItem::lit: {
      static_cast<vLitPipeline*>(pipelines[i].get())->UpdateGlobalUniform(a);
    }
    default:
      break;
    }
  }

  for (int i = 0; i < totalRIs.size(); ++i) {
    vkRenderableItem& ri = *totalRIs[i];
    if (ri._pipeline == RenderableItem::lit) {
      static_cast<vLitPipeline*>(pipelines[i].get())->UpdateModelUniform(a);
    }

    std::function<void(const vk::CommandBuffer&)> f4 = [&ri, a](const vk::CommandBuffer& c) {
      pipelines[ri._pipeline]->BindReleventDescriptor(c, a);
    };

    cmdBuffers->Record(ctx->device, *renderPass, swapchain->swapChainExtent, swapchain->swapChainFramebuffers, *pipelines[ri._pipeline], *ri._vbuffer,
                       ri._icount, f4, a);
  }

  drawFrameInternal(a, ctx->device, ctx->graphicsQueue, ctx->presentQueue, swapchain->swapChain, cmdBuffers->commandBuffers, *syncObjects);
}

void VulkanBackend::resize() {
  // need to recreate framebuffers
  //	vkDeviceWaitIdle(vkinfo->device);
  std::cout << "VK recreate" << std::endl;
}

vkRenderableItem::~vkRenderableItem() { _vbuffer.reset(); }
void vkRenderableItem::updateData(Game::Vertex* vertices, uint32_t vcount, glm::uint16_t* indices, uint32_t icount) {}

vkRenderableItem::vkRenderableItem(Game::Vertex* vertices, uint32_t vcount, glm::uint16_t* indices, uint32_t icount, RenderableItem::PIPELINE p)
    : RenderableItem(vertices, vcount, indices, icount, p) {
  totalRIs.push_back(this);

  std::vector<Vertex> convertedVertexes;
  size_t barrychuckle = 0;

  for (uint32_t j = 0; j < _vcount; ++j) {
    convertedVertexes.push_back(vertices[j]);
  }

  for (uint32_t j = 0; j < _icount; ++j) {
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

void vkRenderableItem::setUniformModelMatrix(glm::mat4 m) {

  //_uniformData.model = m;
  //  _uniformData.mvp = _uniformData.proj * _uniformData.view * m;
}
void vkRenderableItem::updateUniform() {
  // _uniformData.view = Engine::getViewMatrix();
  // _uniformData.proj = Engine::getProjectionMatrix();
  // setUniformModelMatrix(_uniformData.model);
  // _uniformData.pointLight = Engine::getLightPos();
}
