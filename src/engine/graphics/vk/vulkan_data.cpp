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
  // vkBindBufferMemory(device, buffer, devmem, 0);
  std::cout << "Buffer created on device, Size: " << memRequirements.size << " " << devmem << std::endl;

  device.bindBufferMemory(buffer, devmem, 0);
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
  // Todo: check no command buffers have us included
  // This can't be done if there is a refernce to us in any command buffer.
  CmdBuffers::invalidate(this);
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

Uniform::Uniform(size_t uboSize, size_t qty, const vk::Device& device, const vk::PhysicalDevice& physicalDevice)
    : _size{uboSize}, _qty{qty}, _logicalDevice(device) {
  uniformBuffers.resize(_qty);
  uniformBuffersMemory.resize(_qty);

  for (size_t i = 0; i < _qty; i++) {
    uniformBuffers[i] = createBuffer(device, _size, vk::BufferUsageFlagBits::eUniformBuffer);
    uniformBuffersMemory[i] = AllocateBufferOnDevice(
        device, physicalDevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffers[i]);
  }
  std::cout << "Created new set of " << _qty << " uniforms of size: " << _size << std::endl;
}

Uniform::~Uniform() {
  for (size_t i = 0; i < _qty; i++) {
    _logicalDevice.destroyBuffer(uniformBuffers[i]);
    _logicalDevice.freeMemory(uniformBuffersMemory[i]);
  }
}
void Uniform::updateUniformBuffer(uint32_t currentImage, const void* uboData, uint32_t which) {
  auto data = _logicalDevice.mapMemory(uniformBuffersMemory[currentImage], 0, VK_WHOLE_SIZE);
  memcpy(data, uboData, _size);
  _logicalDevice.unmapMemory(uniformBuffersMemory[currentImage]);
}

DescriptorSetLayout::DescriptorSetLayout(const vk::Device& device, const std::vector<vk::DescriptorSetLayoutBinding>& bindings)
    : _logicalDevice(device) {
  vk::DescriptorSetLayoutCreateInfo layoutInfo;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();
  descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
}

DescriptorSetLayout::~DescriptorSetLayout() { _logicalDevice.destroyDescriptorSetLayout(descriptorSetLayout); }


DescriptorPool::DescriptorPool(const vk::Device& device, const std::vector<vk::Image>& swapChainImages) : _logicalDevice{device} {
  vk::DescriptorPoolSize poolSize;
  poolSize.type = vk::DescriptorType::eUniformBuffer;
  poolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());

  std::array<vk::DescriptorPoolSize, VLIT_BINDINGS_COUNT> poolSizes = {};
  poolSizes[vLIT_GLOBAL_UBO_BINDING].type = vk::DescriptorType::eUniformBuffer;
  poolSizes[vLIT_GLOBAL_UBO_BINDING].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
  poolSizes[vLIT_MODEL_UBO_BINDING].type = vk::DescriptorType::eUniformBuffer;
  poolSizes[vLIT_MODEL_UBO_BINDING].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
  poolSizes[vLIT_IMAGE_UBO_BINDING].type = vk::DescriptorType::eCombinedImageSampler;
  poolSizes[vLIT_IMAGE_UBO_BINDING].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

  vk::DescriptorPoolCreateInfo poolInfo;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());
  descriptorPool = device.createDescriptorPool(poolInfo);
}

DescriptorPool::~DescriptorPool() { _logicalDevice.destroyDescriptorPool(descriptorPool); }

TextureImage::TextureImage(const vk::Device& device, const vk::PhysicalDevice& pdevice, const vk::CommandPool& pool, vk::Queue& queue)
    : _logicalDevice(device), _physicalDevice(pdevice), _pool(pool), _queue(queue) {
  int texWidth, texHeight, texChannels;
  const std::string imagename = "gravel09_col_sm.jpg";
  stbi_uc* pixels = stbi_load(std::string("res/" + imagename).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  //upload image data into stagingBuffer
  {
    stagingBuffer = createBuffer(_logicalDevice, imageSize, vk::BufferUsageFlagBits::eTransferSrc);
    stagingBufferMemory =
        AllocateBufferOnDevice(device, pdevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostVisible, stagingBuffer);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);
    stbi_image_free(pixels);
  }

  std::cout << "Loaded image: " << imagename << " " << texWidth << "x" << texHeight << " size:" << imageSize << std::endl;
  //create a new memory area as an image
  createImage(pdevice, texWidth, texHeight, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
              vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, _image,
              _imageMemory);

  //copy from stagingBuffer
  transitionImageLayout(_image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
  copyBufferToImage(stagingBuffer, _image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  transitionImageLayout(_image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

  //delete stagingBuffer
  device.destroyBuffer(stagingBuffer);
  device.freeMemory(stagingBufferMemory);

  // ImageView
  {
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
  }
  // sampler
  {
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_FALSE;
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
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = vk::Offset3D(0, 0, 0);
  region.imageExtent = vk::Extent3D(width, height, 1);

  commandBuffer.commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
  commandBuffer.submit(_queue);
}
