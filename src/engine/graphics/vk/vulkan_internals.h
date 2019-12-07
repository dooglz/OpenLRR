//
// Created by Sam Serrels on 30/11/2019.
//

#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

// things that exist for whole applicaiton
struct ContextInfo {
  ContextInfo();
  ~ContextInfo();
  const vk::Instance instance;
  const vk::SurfaceKHR surface;
  const vk::PhysicalDevice physicalDevice;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;
  vk::Device device; // AKA logical device
  VkDebugUtilsMessengerEXT debugMessenger;

  // It's common that these two things are needed frequently
  struct PhyDevSurfKHR {
    const vk::PhysicalDevice device;
    const vk::SurfaceKHR surface;
  };
  const PhyDevSurfKHR deviceKHR;
};
struct CmdPool {
  vk::CommandPool commandPool;
  CmdPool(const ContextInfo::PhyDevSurfKHR& PhyDevSurf, const vk::Device& device);
  ~CmdPool();

private:
  const vk::Device& _logicalDevice;
};
// Things that change based on render conditions
struct SwapChainInfo {
  SwapChainInfo(const ContextInfo::PhyDevSurfKHR& pds, const vk::Device& logicalDevice);
  ~SwapChainInfo();
  std::vector<vk::Image> swapChainImages;
  vk::Format swapChainImageFormat;
  vk::Extent2D swapChainExtent;
  const vk::SwapchainKHR swapChain;
  const std::vector<vk::ImageView> swapChainImageViews;
  void InitFramebuffers(const vk::RenderPass& renderPass);
  std::vector<vk::Framebuffer> swapChainFramebuffers;

private:
  // A SwapChainInfo can't exist without a device anyway, and it allows us to
  // deconstruct ourtselves
  const vk::Device& _logicalDevice;
};

struct Pipeline {
  vk::PipelineLayout pipelineLayout;
  vk::Pipeline graphicsPipeline;
  Pipeline(const vk::Device& device, const vk::Extent2D& swapChainExtent, const vk::RenderPass& renderPass,
           const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo, vk::DescriptorSetLayout descriptorSetLayout);
  ~Pipeline();

private:
  const vk::Device& _logicalDevice;
};

struct SyncObjects {
  std::vector<vk::Semaphore> imageAvailableSemaphores;
  std::vector<vk::Semaphore> renderFinishedSemaphores;
  std::vector<vk::Fence> inFlightFences;
  std::vector<vk::Fence> imagesInFlight;
  size_t currentFrame = 0;
  static const int MAX_FRAMES_IN_FLIGHT = 2;
  SyncObjects(const vk::Device& device, const size_t swapChainImagesCount);
  ~SyncObjects();

private:
  const vk::Device& _logicalDevice;
};

template <class T> struct VertexDataFormat {
  static vk::PipelineVertexInputStateCreateInfo getPipelineInputState() {

    static vk::PipelineVertexInputStateCreateInfo a(
            vk::PipelineVertexInputStateCreateFlags(),//PipelineVertexInputStateCreateFlags
            1, // vertexBindingDescriptionCount
            T::getBindingDescription(), //VertexInputBindingDescription
            T::getAttributeDescriptions()->size(), //vertexAttributeDescriptionCount
            T::getAttributeDescriptions()->data() //VertexInputAttributeDescription
             );
    int ff = 4;
    return a;
  }
};

struct Vertex : public VertexDataFormat<Vertex> {
  glm::vec2 pos;
  glm::vec3 color;
  Vertex(glm::vec2 p, glm::vec3 c) : pos{p}, color{c} {};

  static const vk::VertexInputBindingDescription* getBindingDescription() {
    static vk::VertexInputBindingDescription b(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
    return &b;
  };
  static const std::array<vk::VertexInputAttributeDescription, 2>* getAttributeDescriptions() {
    const static std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions = { //eR32G32B32A32Sfloat
        vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)),
        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color))};
    return &attributeDescriptions;
  };
};

// Quanity of data is immutable, content can be re-uploaded.
struct VertexBuffer {
  vk::Buffer vertexBuffer;
  vk::DeviceMemory vertexBufferMemory;
  vk::Buffer indexBuffer;
  vk::DeviceMemory indexBufferMemory;
  // total size of data
  const vk::DeviceSize size_vertex;
  const vk::DeviceSize size_index;

  VertexBuffer(const vk::Device& device, const vk::PhysicalDevice& pdevice, const vk::DeviceSize size_vertex, const vk::DeviceSize size_index);
  ~VertexBuffer();

  void UploadVertex(void const* inputdata, size_t uploadSize, const vk::CommandPool& cmdpool, vk::Queue& graphicsQueue);
  void UploadIndex(void const* inputdata, size_t uploadSize, const vk::CommandPool& cmdpool, vk::Queue& graphicsQueue);

  // void CopyBuffer(const vk::CommandPool& cmdpool, const vk::Device& device, vk::Queue& graphicsQueu);
  static void CopyBufferGeneric(const vk::Buffer& srcBuffer, vk::Buffer& dstBuffer, const vk::DeviceSize size, const vk::CommandPool& cmdpool,
                                const vk::Device& device, vk::Queue& graphicsQueue);
  static void UploadGeneric(void const* inputdata, size_t uploadSize, vk::Buffer& buffer, const vk::Device& device,
                            const vk::PhysicalDevice& physicalDevice, const vk::CommandPool& cmdpool, vk::Queue& graphicsQueue);

private:
  const vk::Device& _logicalDevice;
  const vk::PhysicalDevice& _physicalDevice;
};

void drawFrameInternal(uint32_t imageIndex, const vk::Device& device, const vk::Queue& graphicsQueue, const vk::Queue& presentQueue,
                       const VkSwapchainKHR& swapChain, const std::vector<vk::CommandBuffer>& commandBuffers, SyncObjects& sync);
void RebuildSwapChain();
// returns -1 if swapchain needs to be rebuilt.
uint32_t WaitForAvilibleImage(const vk::Device& device, const vk::SwapchainKHR& swapChain, SyncObjects& sync);

// GLFW bridge, TODO make interface headder
VkSurfaceKHR CreateVKWindowSurface(const vk::Instance& instance);
// TODO, wrap in class
vk::UniqueRenderPass createRenderPass(const vk::Device& device, const vk::Format& swapChainImageFormat);

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 mvp;
};

struct Uniform {
  std::vector<vk::Buffer> uniformBuffers;
  std::vector<vk::DeviceMemory> uniformBuffersMemory;
  Uniform(size_t qty, const vk::Device& device, const vk::PhysicalDevice& physicalDevice);
  ~Uniform();
  void updateUniformBuffer(uint32_t currentImage, double dt, const vk::Extent2D& swapChainExtent);

private:
  const vk::Device& _logicalDevice;
  const size_t _qty;
};

struct DescriptorSetLayout {
  vk::DescriptorSetLayout descriptorSetLayout;
  DescriptorSetLayout(const vk::Device& device);
  ~DescriptorSetLayout();

private:
  const vk::Device& _logicalDevice;
};

struct DescriptorPool {
  vk::DescriptorPool descriptorPool;
  DescriptorPool(const vk::Device& device, const std::vector<vk::Image>& swapChainImages);
  ~DescriptorPool();

private:
  const vk::Device& _logicalDevice;
};

struct DescriptorSets {
  std::vector<vk::DescriptorSet> descriptorSets;
  DescriptorSets(const vk::Device& device, const std::vector<vk::Image>& swapChainImages, const vk::DescriptorSetLayout& descriptorSetLayout,
                 const vk::DescriptorPool& descriptorPool, const std::vector<vk::Buffer>& uniformBuffers);
  ~DescriptorSets();

private:
  const vk::Device& _logicalDevice;
};

struct CmdBuffers {
  std::vector<vk::CommandBuffer> commandBuffers;
  CmdBuffers(const vk::Device& device, const vk::CommandPool& pool, size_t amount);
  void Record(const vk::RenderPass& renderPass, const vk::Extent2D& swapChainExtent, const std::vector<vk::Framebuffer>& swapChainFramebuffers,
              const Pipeline& pipeline, const VertexBuffer& vbuf, uint32_t vcount, const DescriptorSets& descriptorSets);

private:
  void RecordCommands(const VertexBuffer& vbuf, uint32_t count, const vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& pipelineLayout,
                      const vk::DescriptorSet& descriptorSet);
};
