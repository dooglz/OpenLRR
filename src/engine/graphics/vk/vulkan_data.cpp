//
// Created by Sam Serrels on 30/11/2019.
//

#include "../../../utils.h"
#include "../../Engine.h"
#include "vulkan.h"
#include "vulkan_internals.h"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <glm/gtx/rotate_vector.hpp>
#include <stb_image.h>
#include <vulkan/vulkan.hpp>
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
  //Todo: check no command buffers have us included
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

void createDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding uboLayoutBinding = {};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;

  // vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

Uniform::Uniform(size_t qty, const vk::Device& device, const vk::PhysicalDevice& physicalDevice) : _qty{qty}, _logicalDevice(device) {
  vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

  uniformBuffers.resize(_qty);
  uniformBuffersMemory.resize(_qty);

  for (size_t i = 0; i < _qty; i++) {
    uniformBuffers[i] = createBuffer(device, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer);
    uniformBuffersMemory[i] = AllocateBufferOnDevice(
        device, physicalDevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffers[i]);
  }
}

Uniform::~Uniform() {
  for (size_t i = 0; i < _qty; i++) {
    _logicalDevice.destroyBuffer(uniformBuffers[i]);
    _logicalDevice.freeMemory(uniformBuffersMemory[i]);
  }
}

void Uniform::updateUniformBuffer(uint32_t currentImage, double dt, const vk::Extent2D& swapChainExtent) {
  // do double for multiplicaiton beacuse CPUs like numbers.

  static double lifetime = 0;
  lifetime += dt;
  double sl = 0.5 + (sin(lifetime) * 0.5);
  // glm::rotate((float)lifetime, glm::vec3(1.f, 0, 0));

  glm::quat q1 = glm::angleAxis(-1.0f, glm::vec3(1.0, 1.0f, 0));
  glm::quat q2 = glm::angleAxis(1.0f, glm::vec3(1.0, 1.0f, 0));
  glm::quat interpolatedquat = glm::mix(q1, q2, (float)sl);
  glm::vec3 gg = normalize(interpolatedquat * glm::vec3(0, 0, 1.f));

  // glm::vec3 directionalLight = gg;
  glm::vec3 directionalLight = gg;
 // directionalLight = glm::vec3(1.0, 1.0f, 0);
  //= glm::rotate(glm::vec3(0, 0, 1.f), (float)lifetime, glm::vec3(1.f, 0, 0));

  // normalize(glm::vec3(0, 0, 1.f));
  glm::dmat4 model = glm::dmat4(1.0);

  glm::dquat cq = Engine::getCamRot();
  glm::dvec3 pos = Engine::getCamPos();
  glm::dvec3 up = normalize(GetUpVector(cq));
  glm::dvec3 forwards = normalize(GetForwardVector(cq));

  glm::dmat4 view = glm::lookAt(pos, pos + forwards, up);

  // auto camera_dir = pos +  glm::rotate(cq, glm::dvec3(1, 0, 0));
  // auto camera_up = glm::rotate(cq, glm::dvec3(0, 0, 1));
  view = glm::mat4_cast(cq) * glm::translate(glm::dmat4(1.0), -pos);
  glm::dmat4 proj = glm::perspective(glm::radians(45.0), (double)swapChainExtent.width / (double)swapChainExtent.height, 0.1, 1000.0);
  // proj[1][1] *= -1;
  glm::dmat4 mvp = proj * view * model;
  // downgrade to float beacuse gpus don't like numbers
  const UniformBufferObject ubo = {(glm::fmat4)model, (glm::fmat4)view, (glm::fmat4)proj, (glm::fmat4)mvp, directionalLight,Engine::getLightPos()};

  void* data = _logicalDevice.mapMemory(uniformBuffersMemory[currentImage], 0, sizeof(ubo));
  memcpy(data, &ubo, sizeof(ubo));
  _logicalDevice.unmapMemory(uniformBuffersMemory[currentImage]);
}

DescriptorSetLayout::DescriptorSetLayout(const vk::Device& device) : _logicalDevice(device) {
  vk::DescriptorSetLayoutBinding uboLayoutBinding;
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex |  vk::ShaderStageFlagBits::eFragment;

  // for image sampler
  vk::DescriptorSetLayoutBinding samplerLayoutBinding;
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

  std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
  vk::DescriptorSetLayoutCreateInfo layoutInfo;
  layoutInfo.bindingCount = bindings.size();
  layoutInfo.pBindings = bindings.data();

  descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
}

DescriptorSetLayout::~DescriptorSetLayout() { _logicalDevice.destroyDescriptorSetLayout(descriptorSetLayout); }

DescriptorPool::DescriptorPool(const vk::Device& device, const std::vector<vk::Image>& swapChainImages) : _logicalDevice{device} {
  vk::DescriptorPoolSize poolSize;
  poolSize.type = vk::DescriptorType::eUniformBuffer;
  poolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());

  std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
  poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
  poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

  vk::DescriptorPoolCreateInfo poolInfo;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());
  descriptorPool = device.createDescriptorPool(poolInfo);
}

DescriptorPool::~DescriptorPool() { _logicalDevice.destroyDescriptorPool(descriptorPool); }

DescriptorSets::DescriptorSets(const vk::Device& device, const std::vector<vk::Image>& swapChainImages,
                               const vk::DescriptorSetLayout& descriptorSetLayout, const vk::DescriptorPool& descriptorPool,
                               const std::vector<vk::Buffer>& uniformBuffers, const vk::ImageView& textureImageView,
                               const vk::Sampler& textureSampler)
    : _logicalDevice(device) {

  std::vector<vk::DescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
  vk::DescriptorSetAllocateInfo allocInfo;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
  allocInfo.pSetLayouts = layouts.data();

  descriptorSets = device.allocateDescriptorSets(allocInfo);

  for (size_t i = 0; i < swapChainImages.size(); i++) {

    vk::DescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    vk::DescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;

    std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {};
    descriptorWrites[0].dstSet = descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[1].dstSet = descriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    device.updateDescriptorSets(descriptorWrites, nullptr);
  }
}

DescriptorSets::~DescriptorSets() {}

TextureImage::TextureImage(const vk::Device& device, const vk::PhysicalDevice& pdevice, const vk::CommandPool& pool, vk::Queue& queue)
    : _logicalDevice(device), _physicalDevice(pdevice), _pool(pool), _queue(queue) {
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load("res/gravel09_col_sm.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  _imageBuf = createBuffer(_logicalDevice, imageSize, vk::BufferUsageFlagBits::eTransferSrc);
  _imageMemory =
      AllocateBufferOnDevice(device, pdevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostVisible, _imageBuf);

  void* data;
  vkMapMemory(device, _imageMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(device, _imageMemory);

  stbi_image_free(pixels);

  createImage(pdevice, texWidth, texHeight, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
              vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, _image,
              _imageMemory);

  transitionImageLayout(_image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
  copyBufferToImage(_imageBuf, _image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  transitionImageLayout(_image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

  vk::ImageViewCreateInfo createInfo;
  createInfo.image = _image;
  createInfo.viewType = vk::ImageViewType::e2D;
  createInfo.format = vk::Format::eR8G8B8A8Unorm;
  createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.levelCount = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.subresourceRange.layerCount = 1;
  _imageView = _logicalDevice.createImageView(createInfo);

  // sampler
  {
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    _imageSampler = device.createSampler(samplerInfo);
  }
}

TextureImage::~TextureImage() {
  _logicalDevice.destroySampler(_imageSampler);
  _logicalDevice.destroyImageView(_imageView);
  _logicalDevice.destroyImage(_image);
  _logicalDevice.freeMemory(_imageMemory);
  _logicalDevice.destroyBuffer(_imageBuf);
}

void TextureImage::createImage(const vk::PhysicalDevice& pdevice, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
                               vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory) {
  vk::ImageCreateInfo imageInfo;
  imageInfo.imageType = vk::ImageType::e2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = vk::ImageLayout::eUndefined;
  imageInfo.usage = usage;
  imageInfo.samples = vk::SampleCountFlagBits::e1; // todo chekc this
  imageInfo.sharingMode = vk::SharingMode::eExclusive;

  image = _logicalDevice.createImage(imageInfo);
  vk::MemoryRequirements memRequirements = _logicalDevice.getImageMemoryRequirements(image);
  vk::MemoryAllocateInfo allocInfo;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, pdevice);

  imageMemory = _logicalDevice.allocateMemory(allocInfo);
  _logicalDevice.bindImageMemory(image, imageMemory, 0);

  // _textureImageView = createImageView(_image, VK_FORMAT_R8G8B8A8_UNORM);
}

void TextureImage::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
  OneOffCmdBuffer commandBuffer(_logicalDevice, _pool);

  vk::ImageMemoryBarrier barrier;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  vk::PipelineStageFlags sourceStage;
  vk::PipelineStageFlags destinationStage;

  if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = (vk::AccessFlagBits)0; // TODO check this
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    sourceStage = vk::PipelineStageFlagBits::eTransfer;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  commandBuffer.commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlagBits(), nullptr, nullptr, barrier);
  commandBuffer.submit(_queue);
}
void TextureImage::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
  OneOffCmdBuffer commandBuffer(_logicalDevice, _pool);

  vk::BufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
  ;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = vk::Offset3D(0, 0, 0);
  region.imageExtent = vk::Extent3D(width, height, 1);


  commandBuffer.commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
  commandBuffer.submit(_queue);
}
