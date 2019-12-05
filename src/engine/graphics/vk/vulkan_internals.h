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
  const VkSurfaceKHR surface;
  const VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  vk::Device device; // AKA logical device
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkDebugUtilsMessengerEXT debugMessenger;

  // It's common that these two things are needed frequently
  struct PhyDevSurfKHR {
    const VkPhysicalDevice device;
    const VkSurfaceKHR surface;
  };
  const PhyDevSurfKHR deviceKHR;
};
struct CmdPool {
  VkCommandPool commandPool;
  CmdPool(const ContextInfo::PhyDevSurfKHR& PhyDevSurf, const vk::Device& device);
  ~CmdPool();

private:
  const vk::Device& _logicalDevice;
};
// Things that change based on render conditions
struct SwapChainInfo {
  SwapChainInfo(const ContextInfo::PhyDevSurfKHR& pds, const vk::Device& logicalDevice);
  ~SwapChainInfo();
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  const VkSwapchainKHR swapChain;
  const std::vector<VkImageView> swapChainImageViews;
  void InitFramebuffers(const VkRenderPass& renderPass);
  std::vector<VkFramebuffer> swapChainFramebuffers;

private:
  // A SwapChainInfo can't exist without a device anyway, and it allows us to
  // deconstruct ourtselves
  const vk::Device& _logicalDevice;
};

struct Pipeline {
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;
  Pipeline(const vk::Device& device, const VkExtent2D& swapChainExtent, const VkRenderPass& renderPass,
           const VkPipelineVertexInputStateCreateInfo& vertexInputInfo);
  ~Pipeline();

private:
  const vk::Device& _logicalDevice;
};



struct CmdBuffers {
  std::vector<vk::CommandBuffer> commandBuffers;
  CmdBuffers(const vk::Device& device, const vk::CommandPool& pool, size_t amount);
  void Record(const VkRenderPass& renderPass, const VkExtent2D& swapChainExtent, const std::vector<VkFramebuffer>& swapChainFramebuffers,
              const VkPipeline& graphicsPipeline, const vk::Buffer& vbuf, uint32_t vcount);

private:
  void RecordCommands(const vk::Buffer& vbuf, uint32_t count, const vk::CommandBuffer& cmdBuffer);
};

struct SyncObjects {
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
  static const int MAX_FRAMES_IN_FLIGHT = 2;
  SyncObjects(const vk::Device& device, const size_t swapChainImagesCount);
  ~SyncObjects();
private:
  const vk::Device& _logicalDevice;
};

// GLFW bridge, TODO make interface headder
VkSurfaceKHR CreateVKWindowSurface(const vk::Instance& instance);
//TODO, wrap in class
vk::UniqueRenderPass createRenderPass(const vk::Device& device, const VkFormat& swapChainImageFormat);

template <class T> struct VertexDataFormat {
  static vk::PipelineVertexInputStateCreateInfo getPipelineInputState() {
    static vk::PipelineVertexInputStateCreateInfo a(vk::PipelineVertexInputStateCreateFlags::Flags(), 1, T::getBindingDescription(),
                                                    T::getAttributeDescriptions()->size(), T::getAttributeDescriptions()->data());

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

struct VertexBuffer {
  vk::Buffer vertexBuffer;
  vk::DeviceMemory vertexBufferMemory;
  VertexBuffer(const vk::Device& device, const vk::PhysicalDevice& pdevice, const vk::DeviceSize size);
  ~VertexBuffer();
  void UploadViaMap(void const* inputdata, size_t uploadSize);
  // const uint32_t count;
  const vk::DeviceSize size;

private:
  const vk::Device& _logicalDevice;
};


void drawFrameInternal(uint32_t imageIndex, const vk::Device& device, const vk::Queue& graphicsQueue, const vk::Queue& presentQueue,
                       const VkSwapchainKHR& swapChain, const std::vector<vk::CommandBuffer>& commandBuffers, SyncObjects& sync);
void RebuildSwapChain();
// returns -1 if swapchain needs to be rebuilt.
uint32_t WaitForAvilibleImage(const vk::Device& device, const VkSwapchainKHR& swapChain, SyncObjects& sync);