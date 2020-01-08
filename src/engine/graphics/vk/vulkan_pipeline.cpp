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

void drawFrameInternal(uint32_t imageIndex, const vk::Device& device, const vk::Queue& graphicsQueue, const vk::Queue& presentQueue,
                       const VkSwapchainKHR& swapChain, const std::vector<vk::CommandBuffer>& commandBuffers, SyncObjects& sync) {

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
  submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

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

vLitPipeline::vLitPipeline(const vk::Device& device, const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass)
    : Pipeline(device, swapChainExtent, renderPass, Vertex::getPipelineInputState(), vLitPipeline_DescriptorSetLayout(device).descriptorSetLayout) {}

  vLitPipeline::~vLitPipeline(){
  _descriptorSets.reset();
  _texture.reset();
  _modelUniform.reset();
  _globalUniform.reset();
}

void vLitPipeline::generatePipelineResources(const vk::PhysicalDevice& pdevice, const std::vector<vk::Image>& swapChainImages,
                                             const std::vector<vk::Framebuffer>& swapChainFramebuffers, const vk::DescriptorPool& descriptorPool,
                                             const vk::CommandPool& pool, vk::Queue& queue) {

  // uniforms
  _globalUniform = std::make_unique<Uniform>(sizeof(vLit_global_UniformBufferObject), swapChainFramebuffers.size(), _logicalDevice, pdevice);
  _modelUniform = std::make_unique<Uniform>(sizeof(vLit_object_UniformBufferObject), swapChainFramebuffers.size(), _logicalDevice, pdevice);
  // images
  _texture = std::make_unique<TextureImage>(_logicalDevice, pdevice, pool, queue);

  _descriptorSets = std::make_unique<vLitPipeline_DescriptorSet>(
      _logicalDevice, swapChainImages, vLitPipeline_DescriptorSetLayout(_logicalDevice).descriptorSetLayout, descriptorPool,
      _globalUniform->uniformBuffers, _modelUniform->uniformBuffers, _texture->_imageView, _texture->_imageSampler);
}

void vLitPipeline::BindReleventDescriptor(const vk::CommandBuffer& cmdBuffer, uint32_t index) {
  assert(_descriptorSets && _descriptorSets->descriptorSets.size() >= index);
  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
                               0, // first set
                               _descriptorSets->descriptorSets[index],
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
  vLit_object_UniformBufferObject uniformData = {};
  uniformData.model = glm::mat4(1.0f);
  uniformData.mvp = glm::mat4(Engine::getProjectionMatrix() * Engine::getViewMatrix() * glm::dmat4(1.0f));
  _modelUniform->updateUniformBuffer(index, (void*)&uniformData);
}
