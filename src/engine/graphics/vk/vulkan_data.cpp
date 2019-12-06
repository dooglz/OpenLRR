//
// Created by Sam Serrels on 30/11/2019.
//

#include "vulkan.h"
#include "vulkan_internals.h"
#include <fstream>
#include <iostream>
#include <vulkan/vulkan.h>

// size = sizeof(vertices[0]) * vertices.size();

uint32_t findMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags& properties, const vk::PhysicalDevice& pdevice) {
  vk::PhysicalDeviceMemoryProperties memProperties;
  pdevice.getMemoryProperties(&memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type!");
}

vk::Buffer createBuffer(const vk::Device& device, const vk::DeviceSize size, vk::BufferUsageFlags usage) {

  vk::BufferCreateInfo bufferInfo;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = vk::SharingMode::eExclusive;
  return device.createBuffer(bufferInfo, nullptr);
}

vk::DeviceMemory AllocateBufferOnDevice(const vk::Device& device, const vk::PhysicalDevice& pdevice, const vk::MemoryPropertyFlags& properties,
                                        const vk::Buffer& buffer) {
  vk::MemoryRequirements memRequirements;
  device.getBufferMemoryRequirements(buffer, &memRequirements);
  vk::MemoryAllocateInfo allocInfo;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, pdevice);
  vk::DeviceMemory devmem = device.allocateMemory(allocInfo);
  vkBindBufferMemory(device, buffer, devmem, 0);
  return devmem;
}

VertexBuffer::VertexBuffer(const vk::Device& device, const vk::PhysicalDevice& pdevice, const vk::DeviceSize size_vertex,
                           const vk::DeviceSize size_index)
    : _logicalDevice{device}, _physicalDevice{pdevice}, size_vertex{size_vertex}, size_index{size_index} {
  vertexBuffer = createBuffer(device, size_vertex, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer);
  vertexBufferMemory = AllocateBufferOnDevice(device, pdevice, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer);
  indexBuffer = createBuffer(device, size_index, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer);
  indexBufferMemory = AllocateBufferOnDevice(device, pdevice, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer);
}

VertexBuffer::~VertexBuffer() {
  vkDestroyBuffer(_logicalDevice, vertexBuffer, nullptr);
  vkFreeMemory(_logicalDevice, vertexBufferMemory, nullptr);
  vkDestroyBuffer(_logicalDevice, indexBuffer, nullptr);
  vkFreeMemory(_logicalDevice, indexBufferMemory, nullptr);
}

void VertexBuffer::UploadGeneric(void const* inputdata, size_t uploadSize, vk::Buffer& buffer, const vk::Device& device,
                                 const vk::PhysicalDevice& physicalDevice, const vk::CommandPool& cmdpool, vk::Queue& graphicsQueue) {
  vk::Buffer stagingBuffer = createBuffer(device, uploadSize, vk::BufferUsageFlagBits::eTransferSrc);
  vk::DeviceMemory stagingBufferMemory = AllocateBufferOnDevice(
      device, physicalDevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer);

  void* data = device.mapMemory(stagingBufferMemory, 0, uploadSize);
  memcpy(data, inputdata, uploadSize);
  device.unmapMemory(stagingBufferMemory);

  CopyBufferGeneric(stagingBuffer, buffer, uploadSize, cmdpool, device, graphicsQueue);

  device.destroyBuffer(stagingBuffer);
  device.freeMemory(stagingBufferMemory);
}

void VertexBuffer::UploadVertex(void const* inputdata, size_t uploadSize, const vk::CommandPool& cmdpool, vk::Queue& graphicsQueue) {
  if (uploadSize != size_vertex) {
    throw std::runtime_error("Yo you got the size wrong bro!");
  }
  UploadGeneric(inputdata, size_vertex, vertexBuffer, _logicalDevice, _physicalDevice, cmdpool, graphicsQueue);
}

void VertexBuffer::UploadIndex(void const* inputdata, size_t uploadSize, const vk::CommandPool& cmdpool, vk::Queue& graphicsQueue) {
  if (uploadSize != size_index) {
    throw std::runtime_error("Yo you got the size wrong bro!");
  }
  UploadGeneric(inputdata, size_index, indexBuffer, _logicalDevice, _physicalDevice, cmdpool, graphicsQueue);
}

void VertexBuffer::CopyBufferGeneric(const vk::Buffer& srcBuffer, vk::Buffer& dstBuffer, const vk::DeviceSize size, const vk::CommandPool& cmdpool,
                                     const vk::Device& device, vk::Queue& graphicsQueue) {
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.level = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandPool = cmdpool;
  allocInfo.commandBufferCount = 1;
  vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  commandBuffer.begin(beginInfo);

  vk::BufferCopy copyRegion;
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = size;
  commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

  commandBuffer.end();

  vk::SubmitInfo submitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  graphicsQueue.submit(submitInfo, nullptr);

  graphicsQueue.waitIdle();
  device.freeCommandBuffers(cmdpool, commandBuffer);
}
