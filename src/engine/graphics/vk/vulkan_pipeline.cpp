//
// Created by Sam Serrels on 30/11/2019.
//
#include "vulkan_pipeline.h"
#include "../../Engine.h"
#include "vulkan.h"
#include "vulkan_internals.h"
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
//#include <vulkan/vulkan.hpp>
uint32_t device_minUniformBufferOffsetAlignment;
uint32_t device_maxDescriptorSetUniformBuffersDynamic;

vk::ShaderModule createShaderModule(const std::vector<char>& code, const vk::Device& device) {
  vk::ShaderModuleCreateInfo createInfo;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
  return device.createShaderModule(createInfo);
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
                   const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo, const std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
    : _logicalDevice{device}, _descriptorSetLayouts{descriptorSetLayouts} {
  const std::string shaderName = "basic";
  auto vertShaderCode = readFile("res/shaders/" + shaderName + ".vert.spv");
  auto fragShaderCode = readFile("res/shaders/" + shaderName + ".frag.spv");

  vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode, device);
  vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode, device);

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

  vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = vk::CompareOp::eLess;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.stencilTestEnable = VK_FALSE;

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
  pipelineLayoutInfo.setLayoutCount = (uint32_t)_descriptorSetLayouts.size();
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

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
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;

  graphicsPipeline = device.createGraphicsPipeline(nullptr, pipelineInfo);
  std::cout << "Pipeline created, Shader: " << shaderName << std::endl;

  device.destroyShaderModule(fragShaderModule);
  fragShaderModule = nullptr;
  device.destroyShaderModule(vertShaderModule);
  vertShaderModule = nullptr;
}

Pipeline::~Pipeline() {
  _descriptorSets.clear();
  for (auto& dsl : _descriptorSetLayouts) {
    _logicalDevice.destroyDescriptorSetLayout(dsl);
  }
  _logicalDevice.destroyPipeline(graphicsPipeline);
  _logicalDevice.destroyPipelineLayout(pipelineLayout);
}

uint32_t WaitForAvilibleImage(const vk::Device& device, const vk::SwapchainKHR& swapChain, SyncObjects& sync) {
  vk::ResultValue<uint32_t> result(vk::Result::eSuccess, 0);
  try {
    result = device.acquireNextImageKHR(swapChain, UINT64_MAX, sync.imageAvailableSemaphores[sync.currentFrame], nullptr);
  } catch (vk::OutOfDateKHRError const& e) {
    result.result = vk::Result::eErrorOutOfDateKHR;
  }
  if (result.result == vk::Result::eErrorOutOfDateKHR) {
    std::cout << "SwapChain out of date" << std::endl;
    // RebuildSwapChain();
    // try now
    return -1;
    // result = device.acquireNextImageKHR(swapChain, UINT64_MAX, sync.imageAvailableSemaphores[sync.currentFrame], nullptr);
  }

  if (result.result != vk::Result::eSuccess /*&& result != VK_SUBOPTIMAL_KHR*/) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }
  return result.value;
}

vLitPipeline::vLitPipeline(const vk::Device& device, const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass)
    : Pipeline(device, swapChainExtent, renderPass, Vertex::getPipelineInputState(), getDescriptorSetLayout(device)), _next_offset{0} {}

vLitPipeline::~vLitPipeline() {
  _texture.reset();
  _modelUniform.reset();
  _globalUniform.reset();
}

const std::vector<vk::DescriptorSetLayout> vLitPipeline::getDescriptorSetLayout(const vk::Device& device) {

  std::cout << "max dynamic buffers: " << device_maxDescriptorSetUniformBuffersDynamic << std::endl;
  vk::DescriptorSetLayoutBinding globalUboLayoutBinding;
  globalUboLayoutBinding.binding = 0;
  globalUboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
  globalUboLayoutBinding.descriptorCount = 1;
  globalUboLayoutBinding.pImmutableSamplers = nullptr;
  globalUboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

  vk::DescriptorSetLayoutBinding modelUboLayoutBinding;
  modelUboLayoutBinding.binding = 0;
  modelUboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
  modelUboLayoutBinding.descriptorCount = 1;
  modelUboLayoutBinding.pImmutableSamplers = nullptr;
  modelUboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

  // for image sampler
  vk::DescriptorSetLayoutBinding samplerLayoutBinding;
  samplerLayoutBinding.binding = 0;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

  vk::DescriptorSetLayoutCreateInfo layoutInfo;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &globalUboLayoutBinding;

  std::vector<vk::DescriptorSetLayout> ret;
  ret.push_back(device.createDescriptorSetLayout(layoutInfo));
  layoutInfo.pBindings = &modelUboLayoutBinding;
  ret.push_back(device.createDescriptorSetLayout(layoutInfo));
  layoutInfo.pBindings = &samplerLayoutBinding;
  ret.push_back(device.createDescriptorSetLayout(layoutInfo));
  return ret;
}

void vLitPipeline::generatePipelineResources(const vk::PhysicalDevice& pdevice, const std::vector<vk::Image>& swapChainImages,
                                             const std::vector<vk::Framebuffer>& swapChainFramebuffers, const vk::DescriptorPool& descriptorPool,
                                             const vk::CommandPool& pool, vk::Queue& queue) {

  // uniforms
  _globalUniform = std::make_unique<Uniform>(sizeof(vLit_global_UniformBufferObject), swapChainFramebuffers.size(), _logicalDevice, pdevice);
  _modelUniform = std::make_unique<PackedUniform<vLit_object_UniformBufferObject>>(static_cast<uint32_t>(swapChainFramebuffers.size()), _bucketSize,
                                                                                   _logicalDevice, pdevice);
  // images
  _texture = std::make_unique<TextureImage>(_logicalDevice, pdevice, pool, queue);

  // DescriptorSets - vLIT_GLOBAL_UBO_BINDING
  {
    std::vector<vk::DescriptorSetLayout> layouts(swapChainImages.size(), _descriptorSetLayouts[vLIT_GLOBAL_UBO_BINDING]);
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();
    auto descriptorSets = _logicalDevice.allocateDescriptorSets(allocInfo);
    for (size_t i = 0; i < swapChainImages.size(); i++) {
      vk::DescriptorBufferInfo bufferInfo1 = {};
      bufferInfo1.buffer = _globalUniform->uniformBuffers[i];
      bufferInfo1.offset = 0;
      bufferInfo1.range = sizeof(vLit_global_UniformBufferObject);

      std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {};
      descriptorWrites[0].dstSet = descriptorSets[i];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo1;
      _logicalDevice.updateDescriptorSets(descriptorWrites, nullptr);
    }
    _descriptorSets.push_back(descriptorSets);
  }
  // DescriptorSets - vLIT_MODEL_UBO_BINDING
  {
    std::vector<vk::DescriptorSetLayout> layouts(swapChainImages.size(), _descriptorSetLayouts[vLIT_MODEL_UBO_BINDING]);
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();
    auto descriptorSets = _logicalDevice.allocateDescriptorSets(allocInfo);
    for (size_t i = 0; i < swapChainImages.size(); i++) {
      vk::DescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer = _modelUniform->uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(vLit_object_UniformBufferObject);

      std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {};
      descriptorWrites[0].dstSet = descriptorSets[i];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      _logicalDevice.updateDescriptorSets(descriptorWrites, nullptr);
    }
    _descriptorSets.push_back(descriptorSets);
  }
  // DescriptorSets - vLIT_IMAGE_UBO_BINDING
  {
    std::vector<vk::DescriptorSetLayout> layouts(swapChainImages.size(), _descriptorSetLayouts[vLIT_IMAGE_UBO_BINDING]);
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();
    auto descriptorSets = _logicalDevice.allocateDescriptorSets(allocInfo);
    for (size_t i = 0; i < swapChainImages.size(); i++) {
      vk::DescriptorImageInfo imageInfo = {};
      imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      imageInfo.imageView = _texture->_imageView;
      imageInfo.sampler = _texture->_imageSampler;

      std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {};
      descriptorWrites[0].dstSet = descriptorSets[i];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pImageInfo = &imageInfo;

      _logicalDevice.updateDescriptorSets(descriptorWrites, nullptr);
    }
    _descriptorSets.push_back(descriptorSets);
  }
}

void vLitPipeline::BindReleventDescriptor(const vk::CommandBuffer& cmdBuffer, uint32_t index, const vkRenderableItem* me) {
  assert(_descriptorSets.size() >= index);
  const size_t baseSize = sizeof(vLit_object_UniformBufferObject);
  const uint32_t size = (uint32_t)(alignedSize(sizeof(vLit_object_UniformBufferObject), device_minUniformBufferOffsetAlignment));
  const uint32_t offset = size * getRIUniformOffset(me);

  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
                               vLIT_GLOBAL_UBO_BINDING,                          // first set
                               1,                                                // descriptorSetCount
                               &_descriptorSets[vLIT_GLOBAL_UBO_BINDING][index], // descriptorSets
                               0,                                                // dynamic offsetcount
                               NULL);                                            // pDynamicOffsets

  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
                               vLIT_MODEL_UBO_BINDING,                          // first set
                               1,                                               // descriptorSetCount
                               &_descriptorSets[vLIT_MODEL_UBO_BINDING][index], // descriptorSets
                               1,                                               // dynamic offsetcount
                               &offset                                          // pDynamicOffsets

  );

  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
                               vLIT_IMAGE_UBO_BINDING,                          // first set
                               1,                                               // descriptorSetCount
                               &_descriptorSets[vLIT_IMAGE_UBO_BINDING][index], // descriptorSets
                               0,                                               // dynamic offsetcount
                               NULL);                                           // pDynamicOffsets
}

void vLitPipeline::prepFrame(uint32_t index) {
  // update global
  vLit_global_UniformBufferObject uniformData = {};
  uniformData.view = Engine::getViewMatrix();
  uniformData.proj = Engine::getProjectionMatrix();
  uniformData.eyePosition = glm::vec4(Engine::getCamPos(), 0);
  uniformData.pointLight = glm::vec4(Engine::getLightPos(), 0);
  _globalUniform->updateUniformBuffer(index, &uniformData);
  // send locals down - should laready be internally updated
  _modelUniform->sendToGpu(index);
}

uint32_t vLitPipeline::getRIUniformOffset(const vkRenderableItem* me) {

  auto search = _uniformRImapping.find(me);
  if (search != _uniformRImapping.end()) {
    return search->second;
  }
  _uniformRImapping[me] = _next_offset;
  _next_offset++;
  if (_next_offset >= _bucketSize) {
    throw std::runtime_error("BAH!");
  }
  return _uniformRImapping[me];
}
void vLitPipeline::updateRIUniform(vkRenderableItem* me, const glm::mat4& m) {
  uint32_t uniformOffset = getRIUniformOffset(me);
  (*_modelUniform)[uniformOffset].model = m;
}
