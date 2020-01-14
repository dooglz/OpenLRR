//
// Created by Sam Serrels on 30/11/2019.
//

#include "vulkan_pipeline.h"
#include "vulkan.h"
#include "vulkan_internals.h"

#include "../../Engine.h"
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <vulkan/vulkan.hpp>
VkShaderModule createShaderModule(const std::vector<char>& code, const vk::Device& device) {
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

// It can't be a string, as there are \0 chars everywhere
static std::vector<char> readFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

Pipeline::Pipeline(const vk::Device& device, const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass,
                   const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo, vk::DescriptorSetLayout descriptorSetLayout)
    : _logicalDevice{device} {
  const std::string shaderName = "basic";
  auto vertShaderCode = readFile("res/shaders/" + shaderName + ".vert.spv");
  auto fragShaderCode = readFile("res/shaders/" + shaderName + ".frag.spv");

  VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, device);
  VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, device);

  vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  vk::Viewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapChainExtent.width;
  viewport.height = (float)swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor = {};
  scissor.offset = vk::Offset2D(0, 0);
  scissor.extent = swapChainExtent;

  vk::PipelineViewportStateCreateInfo viewportState = {};
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  vk::PipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = vk::PolygonMode::eFill;
  // rasterizer.polygonMode = vk::PolygonMode::eLine;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = vk::CullModeFlagBits::eBack;
  // rasterizer.cullMode = vk::CullModeFlagBits::eNone;
  rasterizer.frontFace = vk::FrontFace::eClockwise;
  rasterizer.depthBiasEnable = VK_FALSE;

  vk::PipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

  const vk::PipelineColorBlendAttachmentState colorBlendAttachment(
      false,                   // blendEnable
      vk::BlendFactor::eZero,  // srcColorBlendFactor
      vk::BlendFactor::eZero,  // dstColorBlendFactor
      vk::BlendOp::eAdd,       // colorBlendOp
      vk::BlendFactor::eZero,  // srcAlphaBlendFactor
      vk::BlendFactor::eZero,  // dstAlphaBlendFactor
      vk::BlendOp::eAdd,       // alphaBlendOp
      vk::ColorComponentFlags( // colorWriteMask
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA));

  vk::PipelineColorBlendStateCreateInfo colorBlending;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = vk::LogicOp::eCopy;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

  pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo, nullptr);

  vk::GraphicsPipelineCreateInfo pipelineInfo;
  pipelineInfo.flags = vk::PipelineCreateFlags();
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;

  graphicsPipeline = device.createGraphicsPipeline(nullptr, pipelineInfo);
  std::cout << "Pipeline created, Shader: " << shaderName << std::endl;

  vkDestroyShaderModule(device, fragShaderModule, nullptr);
  vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

Pipeline::~Pipeline() {
  vkDestroyPipeline(_logicalDevice, graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(_logicalDevice, pipelineLayout, nullptr);
}

vk::UniqueRenderPass createRenderPass(const vk::Device& device, const vk::Format& swapChainImageFormat) {

  vk::AttachmentDescription colorAttachment;
  colorAttachment.samples = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp = vk::AttachmentLoadOp::eClear, colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
  colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
  colorAttachment.format = swapChainImageFormat;

  vk::AttachmentReference colorAttachmentRef;
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass;
  subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  vk::RenderPassCreateInfo renderPassInfo;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  return device.createRenderPassUnique(renderPassInfo);
}

void vk_DestoryRenderPass(std::unique_ptr<vk::RenderPass> rp, const vk::Device& device) { device.destroyRenderPass(*rp); }

void SwapChainInfo::InitFramebuffers(const vk::RenderPass& renderPass) {
  swapChainFramebuffers.clear();
  swapChainFramebuffers.resize(swapChainImageViews.size());

  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    VkImageView attachments[] = {swapChainImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    swapChainFramebuffers[i] = _logicalDevice.createFramebuffer(framebufferInfo);
  }
  std::cout << swapChainImageViews.size() << " framebuffers created" << std::endl;
}

uint32_t WaitForAvilibleImage(const vk::Device& device, const vk::SwapchainKHR& swapChain, SyncObjects& sync) {
  uint32_t imageIndex;
  VkResult result =
      vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, sync.imageAvailableSemaphores[sync.currentFrame], VK_NULL_HANDLE, &imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    std::cout << "SwapChain out of date" << std::endl;
    // RebuildSwapChain();
    // try now
    return -1;
    result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, sync.imageAvailableSemaphores[sync.currentFrame], VK_NULL_HANDLE, &imageIndex);
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }
  return imageIndex;
}
vk::DescriptorSetLayout vLitPipeline::_layout;
vLitPipeline::vLitPipeline(const vk::Device& device, const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass)
    : Pipeline(device, swapChainExtent, renderPass, Vertex::getPipelineInputState(), getDescriptorSetLayout(device)) {}

vLitPipeline::~vLitPipeline() {
  _logicalDevice.destroyDescriptorSetLayout(_layout);
  _descriptorSets.clear();
  _texture.reset();
  _modelUniform.reset();
  _globalUniform.reset();
}

const vk::DescriptorSetLayout vLitPipeline::getDescriptorSetLayout(const vk::Device& device) {
 
  if (!_layout) {
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    vk::DescriptorSetLayoutBinding globalUboLayoutBinding;
    globalUboLayoutBinding.binding = vLIT_GLOBAL_UBO_BINDING;
    globalUboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    globalUboLayoutBinding.descriptorCount = 1;
    globalUboLayoutBinding.pImmutableSamplers = nullptr;
    globalUboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding modelUboLayoutBinding;
    modelUboLayoutBinding.binding = vLIT_MODEL_UBO_BINDING;
    modelUboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    modelUboLayoutBinding.descriptorCount = 1;
    modelUboLayoutBinding.pImmutableSamplers = nullptr;
    modelUboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

    // for image sampler
    vk::DescriptorSetLayoutBinding samplerLayoutBinding;
    samplerLayoutBinding.binding = vLIT_IMAGE_UBO_BINDING;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    bindings = {globalUboLayoutBinding, modelUboLayoutBinding, samplerLayoutBinding};
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    _layout = device.createDescriptorSetLayout(layoutInfo);
  }
  return _layout;
}

void vLitPipeline::generatePipelineResources(const vk::PhysicalDevice& pdevice, const std::vector<vk::Image>& swapChainImages,
                                             const std::vector<vk::Framebuffer>& swapChainFramebuffers, const vk::DescriptorPool& descriptorPool,
                                             const vk::CommandPool& pool, vk::Queue& queue) {

  // uniforms
  _globalUniform = std::make_unique<Uniform>(sizeof(vLit_global_UniformBufferObject), swapChainFramebuffers.size(), _logicalDevice, pdevice);
  _modelUniform = std::make_unique<PackedUniform<vLit_object_UniformBufferObject>>(static_cast<uint32_t>(swapChainFramebuffers.size()), _bucketSize,
                                                                                   _logicalDevice, pdevice);
  //_modelUniform = std::make_unique<Uniform>(sizeof(vLit_object_UniformBufferObject), swapChainFramebuffers.size(), _logicalDevice, pdevice);
  // images
  _texture = std::make_unique<TextureImage>(_logicalDevice, pdevice, pool, queue);

  // DescriptorSets
  {
    std::vector<vk::DescriptorSetLayout> layouts(swapChainImages.size(), getDescriptorSetLayout(_logicalDevice));
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    _descriptorSets = _logicalDevice.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < swapChainImages.size(); i++) {
      vk::DescriptorBufferInfo bufferInfo1 = {};
      bufferInfo1.buffer = _globalUniform->uniformBuffers[i];
      bufferInfo1.offset = 0;
      bufferInfo1.range = sizeof(vLit_global_UniformBufferObject);
      vk::DescriptorBufferInfo bufferInfo2 = {};
      bufferInfo2.buffer = _modelUniform->uniformBuffers[i];
      bufferInfo2.offset = 0;
      bufferInfo2.range = sizeof(vLit_object_UniformBufferObject);

      vk::DescriptorImageInfo imageInfo = {};
      imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      imageInfo.imageView = _texture->_imageView;
      imageInfo.sampler = _texture->_imageSampler;

      std::array<vk::WriteDescriptorSet, VLIT_BINDINGS_COUNT> descriptorWrites = {};
      descriptorWrites[vLIT_GLOBAL_UBO_BINDING].dstSet = _descriptorSets[i];
      descriptorWrites[vLIT_GLOBAL_UBO_BINDING].dstBinding = 0;
      descriptorWrites[vLIT_GLOBAL_UBO_BINDING].dstArrayElement = 0;
      descriptorWrites[vLIT_GLOBAL_UBO_BINDING].descriptorType = vk::DescriptorType::eUniformBuffer;
      descriptorWrites[vLIT_GLOBAL_UBO_BINDING].descriptorCount = 1;
      descriptorWrites[vLIT_GLOBAL_UBO_BINDING].pBufferInfo = &bufferInfo1;
      descriptorWrites[vLIT_MODEL_UBO_BINDING].dstSet = _descriptorSets[i];
      descriptorWrites[vLIT_MODEL_UBO_BINDING].dstBinding = 1;
      descriptorWrites[vLIT_MODEL_UBO_BINDING].dstArrayElement = 0;
      descriptorWrites[vLIT_MODEL_UBO_BINDING].descriptorType = vk::DescriptorType::eUniformBuffer;
      descriptorWrites[vLIT_MODEL_UBO_BINDING].descriptorCount = 1;
      descriptorWrites[vLIT_MODEL_UBO_BINDING].pBufferInfo = &bufferInfo2;
      descriptorWrites[vLIT_IMAGE_UBO_BINDING].dstSet = _descriptorSets[i];
      descriptorWrites[vLIT_IMAGE_UBO_BINDING].dstBinding = 2;
      descriptorWrites[vLIT_IMAGE_UBO_BINDING].dstArrayElement = 0;
      descriptorWrites[vLIT_IMAGE_UBO_BINDING].descriptorType = vk::DescriptorType::eCombinedImageSampler;
      descriptorWrites[vLIT_IMAGE_UBO_BINDING].descriptorCount = 1;
      descriptorWrites[vLIT_IMAGE_UBO_BINDING].pImageInfo = &imageInfo;

      _logicalDevice.updateDescriptorSets(descriptorWrites, nullptr);
    }
  }
}

void vLitPipeline::BindReleventDescriptor(const vk::CommandBuffer& cmdBuffer, uint32_t index) {
  assert(_descriptorSets.size() >= index);
  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
                               0, // first set
                               _descriptorSets[index],
                               nullptr // dynamicOffsets
  );
}

void vLitPipeline::UpdateGlobalUniform(uint32_t index) {
  vLit_global_UniformBufferObject uniformData = {};
  uniformData.view = Engine::getViewMatrix();
  uniformData.proj = Engine::getProjectionMatrix();
  uniformData.pointLight = glm::vec4(Engine::getLightPos(), 0);
  uniformData.lightDir = glm::vec4(0.0f);
  _globalUniform->updateUniformBuffer(index, &uniformData);
}

void vLitPipeline::UpdateModelUniform(uint32_t index) {
  (*_modelUniform)[0].model = glm::mat4(1.0f);
  (*_modelUniform)[0].mvp = glm::mat4(Engine::getProjectionMatrix() * Engine::getViewMatrix() * glm::dmat4(1.0f));
  _modelUniform->sendToGpu(index);
}
