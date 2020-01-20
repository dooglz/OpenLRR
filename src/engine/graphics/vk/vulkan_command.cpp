//
// Created by Sam Serrels on 30/11/2019.
//

#include "vulkan.h"
#include "vulkan_internals.h"
#include <fstream>
#include <functional>
#include <iostream>
#include <set>
//#include <vulkan/vulkan.hpp>

void CmdBuffers::RecordCommands(const VertexBuffer& vbuf, uint32_t count, const vk::CommandBuffer& cmdBuffer,
                                const vk::PipelineLayout& pipelineLayout, std::function<void(const vk::CommandBuffer&)> descriptorSetFunc) {

  cmdBuffer.bindVertexBuffers(0, vbuf.vertexBuffer, {0});
  cmdBuffer.bindIndexBuffer(vbuf.indexBuffer, 0, vk::IndexType::eUint16);
  descriptorSetFunc(cmdBuffer);
  cmdBuffer.drawIndexed(count, 1, 0, 0, 0);
}

template <typename T, typename... Rest> void hash_combine(std::size_t& seed, const T& v, const Rest&... rest) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hash_combine(seed, rest), ...);
}

struct RecordInfo {
  const vk::RenderPass& renderPass;
  const Pipeline& pipeline;
  const vk::Extent2D& swapChainExtent;
  const std::vector<CmdBuffers::RenderableToken> tokens;
  uint32_t index;
};
struct RecordInfoHash {
  std::size_t operator()(RecordInfo const& s) const noexcept {
    std::size_t h = 0;
    hash_combine(h, static_cast<const void*>(&s.renderPass), static_cast<const void*>(&s.pipeline), static_cast<const void*>(&s.swapChainExtent),
                 s.index, s.tokens.size());
    for (const auto& t : s.tokens) {
      hash_combine(h, static_cast<const void*>(&t.vbuf), t.vcount);
    }
    return h;
  }
};

void CmdBuffers::Record(const vk::Device& device, const vk::RenderPass& renderPass, const vk::Extent2D& swapChainExtent,
                        const std::vector<vk::Framebuffer>& swapChainFramebuffers, const Pipeline& pipeline, std::vector<RenderableToken> tokens,
                        uint32_t index) {

  const RecordInfo h = {renderPass, pipeline, swapChainExtent, tokens, index};
  const size_t thehash = RecordInfoHash{}(h);

  if (commandBuffers[index].hashedState == thehash) {
    return;
  }

  commandBuffers[index].hashedState = thehash;
  commandBuffers[index].referencedVB.clear();
  for (const auto& t : tokens) {
    commandBuffers[index].referencedVB.insert(&t.vbuf);
  }

  const vk::CommandBuffer& cb = commandBuffers[index].commandBuffer;
  std::cout << "commandBuffer " << static_cast<VkCommandBuffer>(cb) << " Recording" << std::endl;
  // commandBuffers[index].reset(vk::CommandBufferResetFlags());
  // commandBufferStates[index] = &pipeline;

  vk::CommandBufferBeginInfo beginInfo;
  cb.begin(beginInfo);

  vk::RenderPassBeginInfo renderPassInfo;
  renderPassInfo.renderPass = renderPass;
  renderPassInfo.framebuffer = swapChainFramebuffers[index];
  renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
  renderPassInfo.renderArea.extent = swapChainExtent;
  vk::ClearValue clearColor = vk::ClearValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;
  cb.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline);

  // RecordCommands(vbuf, vcount, commandBuffers[index], pipeline.pipelineLayout, descriptorSets.descriptorSets[index]);
  for (const auto& t : tokens) {
    RecordCommands(t.vbuf, t.vcount, cb, pipeline.pipelineLayout, t.descriptorSetFunc);
  }
  cb.endRenderPass();
  cb.end();
}
void CmdBuffers::Record(const vk::Device& device, const vk::RenderPass& renderPass, const vk::Extent2D& swapChainExtent,
                        const std::vector<vk::Framebuffer>& swapChainFramebuffers, const Pipeline& pipeline, const VertexBuffer& vbuf,
                        uint32_t vcount, std::function<void(const vk::CommandBuffer&)> descriptorSetFunc, uint32_t index) {
  return Record(device, renderPass, swapChainExtent, swapChainFramebuffers, pipeline, {{vbuf, vcount, descriptorSetFunc}}, index);
}

void CmdBuffers::commandBufferCollection::Reset(const vk::Device& device) {
  if (fence != nullptr) {
    device.waitForFences(1, fence, VK_TRUE, UINT64_MAX);
  }
  commandBuffer.reset(vk::CommandBufferResetFlags());
  referencedVB.clear();
  hashedState = 0;
}

void CmdBuffers::invalidate(const VertexBuffer* vbuf) {
  for (auto cbuffer : _all) {
    for (auto& cbufferC : cbuffer->commandBuffers) {
      if (cbufferC.referencedVB.find(vbuf) != cbufferC.referencedVB.end()) {
        cbufferC.Reset(cbuffer->_logicalDevice);
        std::cout << "commandBuffer " << static_cast<VkCommandBuffer>(cbufferC.commandBuffer) << " Invlalidated" << std::endl;
      }
    }
  }
}

std::set<CmdBuffers*> CmdBuffers::_all;
CmdBuffers::~CmdBuffers() { _all.erase(this); }

CmdBuffers::CmdBuffers(const vk::Device& device, const vk::CommandPool& pool, size_t amount) : _logicalDevice{device} {
  commandBuffers.resize(amount);
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.commandPool = pool;
  allocInfo.level = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
  std::vector<vk::CommandBuffer> buffers = device.allocateCommandBuffers(allocInfo);
  for (size_t i = 0; i < amount; i++) {
    commandBuffers[i].commandBuffer = buffers[i];
  }
  _all.insert(this);
}

OneOffCmdBuffer::OneOffCmdBuffer(const vk::Device& device, const vk::CommandPool& pool) : _logicalDevice(device), _pool(pool) {
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.commandPool = pool;
  allocInfo.level = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandBufferCount = 1;
  commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

  vk::FenceCreateInfo fenceInfo;
  fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
  _fence = device.createFence(fenceInfo);
  device.resetFences(_fence);
  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  commandBuffer.begin(beginInfo);
}

void OneOffCmdBuffer::submit(vk::Queue& queue) {
  commandBuffer.end();
  vk::SubmitInfo submitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  queue.submit(1, &submitInfo, _fence);
  _logicalDevice.waitForFences(1, &_fence, VK_TRUE, UINT64_MAX);
}
OneOffCmdBuffer::~OneOffCmdBuffer() {
  _logicalDevice.destroyFence(_fence);
  _logicalDevice.freeCommandBuffers(_pool, commandBuffer);
}

void drawFrameInternal(uint32_t imageIndex, const vk::Device& device, const vk::Queue& graphicsQueue, const vk::Queue& presentQueue,
                       const vk::SwapchainKHR& swapChain, CmdBuffers::commandBufferCollection& commandBuffer, SyncObjects& sync) {

  device.waitForFences(1, &sync.inFlightFences[sync.currentFrame], VK_TRUE, UINT64_MAX);

  // uint32_t imageIndex = WaitForAvilibleImage(device, swapChain, sync);

  if (sync.imagesInFlight[imageIndex]) {
    device.waitForFences(1, &sync.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
  }
  sync.imagesInFlight[imageIndex] = sync.inFlightFences[sync.currentFrame];

  vk::SubmitInfo submitInfo;
  vk::Semaphore waitSemaphores[] = {sync.imageAvailableSemaphores[sync.currentFrame]};
  vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer.commandBuffer;
  commandBuffer.fence = &sync.inFlightFences[sync.currentFrame];
  vk::Semaphore signalSemaphores[] = {sync.renderFinishedSemaphores[sync.currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  device.resetFences(1, &sync.inFlightFences[sync.currentFrame]);
  graphicsQueue.submit(1, &submitInfo, sync.inFlightFences[sync.currentFrame]);

  vk::PresentInfoKHR presentInfo;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;
  vk::SwapchainKHR swapChains[] = {swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  presentQueue.presentKHR(&presentInfo);

  sync.currentFrame = (sync.currentFrame + 1) % SyncObjects::MAX_FRAMES_IN_FLIGHT;
}
