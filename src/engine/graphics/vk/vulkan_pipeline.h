//
// Created by Sam Serrels on 28/12/2019.
//

#ifndef OPENLRR_VULKAN_PIPELINE_H
#define OPENLRR_VULKAN_PIPELINE_H
#include <vulkan/vulkan.hpp>
#include <glm/fwd.hpp>
#include <map>

struct DescriptorSets;
struct Uniform;
struct TextureImage;
struct vkRenderableItem;
template <typename T> class PackedUniform;
struct vLit_object_UniformBufferObject;


enum VLIT_BINDINGS { vLIT_GLOBAL_UBO_BINDING, vLIT_MODEL_UBO_BINDING, vLIT_IMAGE_UBO_BINDING, VLIT_BINDINGS_COUNT };

struct Pipeline {
  vk::PipelineLayout pipelineLayout;
  vk::Pipeline graphicsPipeline;

  virtual void generatePipelineResources(const vk::PhysicalDevice& pdevice, const std::vector<vk::Image>& swapChainImages,
                                    const std::vector<vk::Framebuffer>& swapChainFramebuffers, const vk::DescriptorPool& descriptorPool,
                                    const vk::CommandPool& pool, vk::Queue& queue) = 0;
  virtual void BindReleventDescriptor(const vk::CommandBuffer& cmdbuff, uint32_t index) = 0;
  //virtual void registerRI(std::shared_ptr<vkRenderableItem> ri) = 0;
  virtual ~Pipeline();
  virtual void prepFrame(uint32_t index) = 0;
  //possibly shouldn't be here
  virtual void updateRIUniform(vkRenderableItem* me, const glm::mat4& m) = 0;

protected:
  const vk::Device& _logicalDevice;
  std::vector<vk::DescriptorSet> _descriptorSets;
  //DescriptorSetLayouts never change.
  const vk::DescriptorSetLayout _descriptorlayout;

  Pipeline(const vk::Device& device, const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass,
           const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo, vk::DescriptorSetLayout descriptorSetLayout);

};

class vLitPipeline : public Pipeline {
public:
  vLitPipeline(const vk::Device& device, const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass);
  ~vLitPipeline();

  void generatePipelineResources(const vk::PhysicalDevice& pdevice, const std::vector<vk::Image>& swapChainImages,
                                 const std::vector<vk::Framebuffer>& swapChainFramebuffers, const vk::DescriptorPool& descriptorPool,
                                 const vk::CommandPool& pool, vk::Queue& queue) override;

  void BindReleventDescriptor(const vk::CommandBuffer& cmdbuff, uint32_t index) override;

  void prepFrame(uint32_t index) override;

  void updateRIUniform(vkRenderableItem* me, const glm::mat4& m) override;
  // void UpdateGlobalUniform(uint32_t index);
  // void UpdateModelUniform(uint32_t index);
  // void registerRI(std::shared_ptr<vkRenderableItem> ri) override;

  // static std::vector<std::shared_ptr<vkRenderableItem>> registeredRIs;

protected:
  const uint32_t _bucketSize = 16;
  static const vk::DescriptorSetLayout getDescriptorSetLayout(const vk::Device& device);
  // const vk::DescriptorSetLayout _descriptorSetLayout;
  // std::unique_ptr<DescriptorSets> _descriptorSets;
  std::unique_ptr<Uniform> _globalUniform;
  std::unique_ptr<PackedUniform<vLit_object_UniformBufferObject>> _modelUniform;
  std::unique_ptr<TextureImage> _texture;
  uint32_t getRIUniformOffset(vkRenderableItem*);
  std::map<vkRenderableItem*, uint32_t> _uniformRImapping;
};
#endif // OPENLRR_VULKAN_PIPELINE_H
