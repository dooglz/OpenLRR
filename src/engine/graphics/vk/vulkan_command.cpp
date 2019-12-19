//
// Created by Sam Serrels on 30/11/2019.
//

#include "vulkan.h"
#include "vulkan_internals.h"
#include <fstream>
#include <iostream>
#include <set>
#include <vulkan/vulkan.hpp>

void CmdBuffers::RecordCommands(const VertexBuffer& vbuf, uint32_t count, const vk::CommandBuffer& cmdBuffer,
                                const vk::PipelineLayout& pipelineLayout, const vk::DescriptorSet& descriptorSet) {

  cmdBuffer.bindVertexBuffers(0, vbuf.vertexBuffer, {0});
  cmdBuffer.bindIndexBuffer(vbuf.indexBuffer, 0, vk::IndexType::eUint16);

  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
                               0, // first set
                               descriptorSet,
                               nullptr // dynamicOffsets
  );
  vkCmdDrawIndexed(cmdBuffer, count, 1, 0, 0, 0);

  // vkCmdDraw(cmdBuffer, count, 1, 0, 0);
}

template <typename T, typename... Rest> void hash_combine(std::size_t& seed, const T& v, const Rest&... rest) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hash_combine(seed, rest), ...);
}

struct RecordInfo {
  const vk::RenderPass& renderPass;
  const Pipeline& pipeline;
  const vk::Extent2D& swapChainExtent;
  const VertexBuffer& vbuf;
  uint32_t vcount, index;
};
struct RecordInfoHash {
  std::size_t operator()(RecordInfo const& s) const noexcept {
    std::size_t h = 0;
    hash_combine(h, (void*)s.renderPass, (void*)&s.pipeline, (void*)&s.swapChainExtent, (void*)&s.vbuf, s.vcount, s.index);
    return h;
  }
};

void CmdBuffers::Record(const vk::Device& device, const vk::RenderPass& renderPass, const vk::Extent2D& swapChainExtent,
                        const std::vector<vk::Framebuffer>& swapChainFramebuffers, const Pipeline& pipeline, const VertexBuffer& vbuf,
                        uint32_t vcount, const DescriptorSets& descriptorSets, uint32_t index) {

  const RecordInfo h = {renderPass, pipeline, swapChainExtent, vbuf, vcount, index};
  const size_t thehash = RecordInfoHash{}(h);

  if (commandBufferStates[index] == thehash) {
    return;
  }

  commandBufferStates[index] = thehash;

  // commandBuffers[index].reset(vk::CommandBufferResetFlags());
  // commandBufferStates[index] = &pipeline;

  vk::CommandBufferBeginInfo beginInfo;
  commandBuffers[index].begin(beginInfo);

  vk::RenderPassBeginInfo renderPassInfo;
  renderPassInfo.renderPass = renderPass;
  renderPassInfo.framebuffer = swapChainFramebuffers[index];
  renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
  renderPassInfo.renderArea.extent = swapChainExtent;
  vk::ClearValue clearColor = vk::ClearValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;
  commandBuffers[index].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  commandBuffers[index].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline);

  RecordCommands(vbuf, vcount, commandBuffers[index], pipeline.pipelineLayout, descriptorSets.descriptorSets[index]);

  commandBuffers[index].endRenderPass();
  commandBuffers[index].end();
}

CmdBuffers::CmdBuffers(const vk::Device& device, const vk::CommandPool& pool, size_t amount) {
  commandBuffers.resize(amount);
  commandBufferStates.resize(amount);
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.commandPool = pool;
  allocInfo.level = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
  commandBuffers = device.allocateCommandBuffers(allocInfo);
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
