//
// Created by Sam Serrels on 30/11/2019.
//

#include "vulkan.h"
#include "vulkan_internals.h"
#include <fstream>
#include <iostream>
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

void CmdBuffers::Record(const vk::RenderPass& renderPass, const vk::Extent2D& swapChainExtent,
                        const std::vector<vk::Framebuffer>& swapChainFramebuffers, const Pipeline& pipeline, const VertexBuffer& vbuf,
                        uint32_t vcount, const DescriptorSets& descriptorSets) {
  for (size_t i = 0; i < commandBuffers.size(); i++) {

    vk::CommandBufferBeginInfo beginInfo;
    commandBuffers[i].begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[i];
    renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassInfo.renderArea.extent = swapChainExtent;
    vk::ClearValue clearColor = vk::ClearValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline);

    RecordCommands(vbuf, vcount, commandBuffers[i], pipeline.pipelineLayout, descriptorSets.descriptorSets[i]);

    commandBuffers[i].endRenderPass();
    commandBuffers[i].end();
  }
}

CmdBuffers::CmdBuffers(const vk::Device& device, const vk::CommandPool& pool, size_t amount) {
  commandBuffers.resize(amount);
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.commandPool = pool;
  allocInfo.level = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
  commandBuffers = device.allocateCommandBuffers(allocInfo);
}
