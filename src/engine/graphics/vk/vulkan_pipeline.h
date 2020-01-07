//
// Created by Sam Serrels on 28/12/2019.
//

#ifndef OPENLRR_VULKAN_PIPELINE_H
#define OPENLRR_VULKAN_PIPELINE_H
#include <vulkan/vulkan.hpp>
struct DescriptorSets;
struct Uniform;
struct TextureImage;

enum VLIT_BINDINGS { vLIT_GLOBAL_UBO_BINDING, vLIT_MODEL_UBO_BINDING, vLIT_IMAGE_UBO_BINDING, VLIT_BINDINGS_COUNT };

struct Pipeline {
  vk::PipelineLayout pipelineLayout;
  vk::Pipeline graphicsPipeline;
  Pipeline(const vk::Device& device, const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass,
           const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo, vk::DescriptorSetLayout descriptorSetLayout);
  ~Pipeline();
  virtual void BindReleventDescriptor(const vk::CommandBuffer& cmdbuff, uint32_t index){};

protected:
  const vk::Device& _logicalDevice;
};

class vLitPipeline : public Pipeline {
public:
  vLitPipeline(const vk::Device& device, const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass);
  void generatePipelineResources(const vk::PhysicalDevice& pdevice, const std::vector<vk::Image>& swapChainImages,
                                 const std::vector<vk::Framebuffer>& swapChainFramebuffers, const vk::DescriptorPool& descriptorPool,
                                 const vk::CommandPool& pool, vk::Queue& queue);
  void BindReleventDescriptor(const vk::CommandBuffer& cmdbuff, uint32_t index) override;
  void UpdateGlobalUniform(uint32_t index);
  void UpdateModelUniform(uint32_t index);

private:
  // const vk::DescriptorSetLayout _descriptorSetLayout;
  // std::unique_ptr<DescriptorSets> _descriptorSets;

  std::unique_ptr<DescriptorSets> _descriptorSets;
  std::unique_ptr<Uniform> _globalUniform;
  std::unique_ptr<Uniform> _modelUniform;
  std::unique_ptr<TextureImage> _texture;
};

#endif // OPENLRR_VULKAN_PIPELINE_H
