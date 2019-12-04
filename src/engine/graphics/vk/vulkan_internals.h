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
  VkDevice device; // AKA logical device
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

// Things that change based on render conditions
struct SwapChainInfo {
  SwapChainInfo(const ContextInfo::PhyDevSurfKHR& pds, const VkDevice& logicalDevice);
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
  const VkDevice& _logicalDevice;
};

struct Pipeline {
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;
  Pipeline(const VkDevice& device, const VkExtent2D& swapChainExtent, const VkRenderPass& renderPass,
           const VkPipelineVertexInputStateCreateInfo& vertexInputInfo);
  ~Pipeline();

private:
  const VkDevice& _logicalDevice;
};

struct CmdPoolBuf {
  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;
  CmdPoolBuf(const ContextInfo::PhyDevSurfKHR& PhyDevSurf, const VkDevice& device, const VkRenderPass& renderPass, const VkPipeline& graphicsPipeline,
             const std::vector<VkFramebuffer>& swapChainFramebuffers, const VkExtent2D& swapChainExtent);
  ~CmdPoolBuf();

private:
  const VkDevice& _logicalDevice;
};

struct SyncObjects {
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
  static const int MAX_FRAMES_IN_FLIGHT = 2;
  SyncObjects(const VkDevice& device, const std::vector<VkImage>& swapChainImages);
  ~SyncObjects();

private:
  const VkDevice& _logicalDevice;
};

// GLFW bridge, TODO make interface header
VkSurfaceKHR CreateVKWindowSurface(const vk::Instance& instance);
vk::UniqueRenderPass createRenderPass(const vk::Device& device, const VkFormat& swapChainImageFormat);

void drawFrameInternal(uint32_t imageIndex, const VkDevice& device, const VkQueue& graphicsQueue, const VkQueue& presentQueue,
                       const VkSwapchainKHR& swapChain, const std::vector<VkCommandBuffer>& commandBuffers, SyncObjects& sync);
void RebuildSwapChain();
// returns -1 if swapchain needs to be rebuilt.
uint32_t WaitForAvilibleImage(const VkDevice& device, const VkSwapchainKHR& swapChain, SyncObjects& sync);

template <class T> struct VertexDataFormat {
  static vk::PipelineVertexInputStateCreateInfo getPipelineInputState() {
    static vk::PipelineVertexInputStateCreateInfo a(
		vk::PipelineVertexInputStateCreateFlags::Flags(), 1, T::getBindingDescription(),
                                                    T::getAttributeDescriptions()->size(), T::getAttributeDescriptions()->data());

    return a;
  }
};

struct Vertex : public VertexDataFormat<Vertex> {
  glm::vec2 pos;
  glm::vec3 color;
  Vertex(glm::vec2 p, glm::vec3 c) : pos{p}, color{c} {};

  static const  vk::VertexInputBindingDescription* getBindingDescription() {
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
  VertexBuffer(const vk::Device& device, const vk::PhysicalDevice& pdevice, const vk::DeviceSize size, const uint32_t count);
  ~VertexBuffer();

  void UploadViaMap(void const* inputdata);

   const uint32_t count;
  const vk::DeviceSize size;
private:
  const VkDevice& _logicalDevice;

};