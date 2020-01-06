//
// Created by Sam Serrels on 28/12/2019.
//

#ifndef OPENLRR_VULKAN_PIPELINE_H
#define OPENLRR_VULKAN_PIPELINE_H
#include <vulkan/vulkan.hpp>


struct Pipeline {
  vk::PipelineLayout pipelineLayout;
  vk::Pipeline graphicsPipeline;
  Pipeline(const vk::Device& device, const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass,
           const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo, vk::DescriptorSetLayout descriptorSetLayout);
  ~Pipeline();

private:

  const vk::Device& _logicalDevice;
};



class vLitPipeline : public Pipeline{
  vLitPipeline(const vk::Device& device, const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass);
private:
  vk::DescriptorSetLayout _descriptorSetLayout;
};

#endif // OPENLRR_VULKAN_PIPELINE_H
