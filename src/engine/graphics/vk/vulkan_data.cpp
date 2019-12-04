//
// Created by Sam Serrels on 30/11/2019.
//

#include "vulkan.h"
#include "vulkan_internals.h"
#include <fstream>
#include <iostream>
#include <vulkan/vulkan.h>

const std::vector<Vertex> vertices = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
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

VertexBuffer::VertexBuffer(const vk::Device& device, const vk::PhysicalDevice& pdevice, const vk::DeviceSize size)
    : _logicalDevice(device), size{size} {
  vk::BufferCreateInfo bufferInfo;
  bufferInfo.size = size;
  bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
  bufferInfo.sharingMode = vk::SharingMode::eExclusive;

  if (device.createBuffer(&bufferInfo, nullptr, &vertexBuffer) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  vk::MemoryRequirements memRequirements;
  device.getBufferMemoryRequirements(vertexBuffer, &memRequirements);
  vk::MemoryAllocateInfo allocInfo;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, pdevice);

  if (device.allocateMemory(&allocInfo, nullptr, &vertexBufferMemory) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }

  device.bindBufferMemory(vertexBuffer, vertexBufferMemory, 0);
}

VertexBuffer::~VertexBuffer() {
  vkDestroyBuffer(_logicalDevice, vertexBuffer, nullptr);
  vkFreeMemory(_logicalDevice, vertexBufferMemory, nullptr);
}

void VertexBuffer::UploadViaMap(void const* inputdata) {
  void* data;
  vkMapMemory(_logicalDevice, vertexBufferMemory, 0, size, 0, &data);
  memcpy(data, vertices.data(), (size_t)size);
  vkUnmapMemory(_logicalDevice, vertexBufferMemory);
}
