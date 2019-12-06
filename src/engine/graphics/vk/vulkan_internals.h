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
           const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo);
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

// GLFW bridge, TODO make interface headder
VkSurfaceKHR CreateVKWindowSurface(const vk::Instance& instance);
// TODO, wrap in class
vk::UniqueRenderPass createRenderPass(const vk::Device& device, const vk::Format& swapChainImageFormat);

template <class T> struct VertexDataFormat {
  static vk::PipelineVertexInputStateCreateInfo getPipelineInputState() {
    static vk::PipelineVertexInputStateCreateInfo a(vk::PipelineVertexInputStateCreateFlags(), 1, T::getBindingDescription(),
                                                    T::getAttributeDescriptions()->size(), T::getAttributeDescriptions()->data());
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
    const static std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions = {
        vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)),
        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color))};
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

struct CmdBuffers {
  std::vector<vk::CommandBuffer> commandBuffers;
  CmdBuffers(const vk::Device& device, const vk::CommandPool& pool, size_t amount);
  void Record(const vk::RenderPass& renderPass, const vk::Extent2D& swapChainExtent, const std::vector<vk::Framebuffer>& swapChainFramebuffers,
              const vk::Pipeline& graphicsPipeline, const VertexBuffer& vbuf, uint32_t vcount);

private:
  void RecordCommands(const VertexBuffer& vbuf, uint32_t count, const vk::CommandBuffer& cmdBufferr);
};
