//
// Created by Sam Serrels on 30/11/2019.
//

#pragma once

#include "../../../game/game_graphics.h"
#include <functional>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <set>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "vulkan_pipeline.h"

/*
struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 mvp;
  glm::vec3 lightDir;
  glm::vec3 pointLight;
};
*/

struct vLit_global_UniformBufferObject {
  glm::mat4 view;
  glm::mat4 proj;
  glm::vec4 eyePosition;
  glm::vec4 pointLight;
};

struct vLit_object_UniformBufferObject {
  glm::mat4 model;
};

const bool ENABLE_VSYNC = false;
const bool FORCE_FLUSH = true; // Flush anyway, even if we don't need to, because osx.

extern uint32_t device_minUniformBufferOffsetAlignment;
extern uint32_t device_maxDescriptorSetUniformBuffersDynamic;

inline size_t alignedSize(size_t sz, size_t align) { return ((sz + align - 1) / align) * align; }

vk::Buffer createBuffer(const vk::Device& device, const vk::DeviceSize size, vk::BufferUsageFlags usage);

vk::DeviceMemory AllocateBufferOnDevice(const vk::Device& device, const vk::PhysicalDevice& pdevice, const vk::MemoryPropertyFlags& properties,
                                        const vk::Buffer& buffer, vk::DeviceSize* ds_ret = nullptr);

// things that exist for whole applicaiton
struct ContextInfo {
  ContextInfo();

  ~ContextInfo();
  vk::DebugUtilsMessengerEXT debugMessenger;
  vk::Instance instance;
  vk::SurfaceKHR surface;
  vk::PhysicalDevice physicalDevice;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;
  vk::Device device; // AKA logical device

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
  SwapChainInfo(const ContextInfo::PhyDevSurfKHR& pds, const vk::Device& logicalDevice, const vk::RenderPass& renderPass);
  ~SwapChainInfo();

  static vk::SurfaceFormatKHR getImageFormat(const ContextInfo::PhyDevSurfKHR& pds);
  static vk::Format getDepthFormat(const vk::PhysicalDevice& physicalDevice);

  std::vector<vk::Image> swapChainImages;
  std::vector<vk::Framebuffer> swapChainFramebuffers;
  vk::SwapchainKHR swapChain;
  vk::Extent2D swapChainExtent;

protected:
  std::vector<vk::ImageView> swapChainImageViews;
  vk::Image depthImage;
  vk::ImageView depthImageView;
  vk::DeviceMemory depthImageMemory;
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

    static vk::PipelineVertexInputStateCreateInfo a(vk::PipelineVertexInputStateCreateFlags(), // PipelineVertexInputStateCreateFlags
                                                    1,                                         // vertexBindingDescriptionCount
                                                    T::getBindingDescription(),                // VertexInputBindingDescription
                                                    static_cast<uint32_t>(T::getAttributeDescriptions()->size()), // vertexAttributeDescriptionCount
                                                    T::getAttributeDescriptions()->data()                         // VertexInputAttributeDescription
    );
    int ff = 4;
    return a;
  }
};

struct Vertex : public VertexDataFormat<Vertex> {
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec3 normal;
  glm::vec3 barry;

  // Vertex(glm::vec2 p, glm::vec3 c) : pos{p}, color{c} {};
  Vertex(glm::vec3 p, glm::vec3 c, glm::vec3 n = glm::vec3(0.f), glm::vec3 b = glm::vec3(0.f)) : pos{p}, color{c}, normal{n}, barry{b} {};

  Vertex(const Game::Vertex& v) : Vertex{v.p, v.c, v.n} {};

  static const vk::VertexInputBindingDescription* getBindingDescription() {
    static vk::VertexInputBindingDescription b(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
    return &b;
  };

  static const std::array<vk::VertexInputAttributeDescription, 4>* getAttributeDescriptions() {
    const static std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions = {
        // eR32G32B32A32Sfloat
        vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
        vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)),
        vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, barry)),

    };
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

void RebuildSwapChain();

// returns -1 if swapchain needs to be rebuilt.
uint32_t WaitForAvilibleImage(const vk::Device& device, const vk::SwapchainKHR& swapChain, SyncObjects& sync);

// GLFW bridge, TODO make interface headder
VkSurfaceKHR CreateVKWindowSurface(vk::Instance& instance);

// TODO, wrap in class
vk::UniqueRenderPass createRenderPass(const vk::Device& device, const vk::Format& swapChainImageFormat, const vk::Format& swapChainDepthFormat);

struct Uniform {
  std::vector<vk::Buffer> uniformBuffers;
  std::vector<vk::DeviceMemory> uniformBuffersMemory;

  Uniform(size_t uboSize, size_t qty, const vk::Device& device, const vk::PhysicalDevice& physicalDevice);

  ~Uniform();

  void updateUniformBuffer(uint32_t currentImage, const void* uboData, uint32_t which = 0);

private:
  const vk::Device& _logicalDevice;
  const size_t _qty;
  const size_t _size;
};

template <typename T> class PackedUniform {
public:
  PackedUniform(uint32_t framebuffer_sets, uint32_t qty, const vk::Device& device, const vk::PhysicalDevice& physicalDevice)
      : _esize{sizeof(T)}, _qty{qty}, _totalSize{_esize * _qty}, _framebuffer_sets{framebuffer_sets},
        _logicalDevice(device), _esizeDevice{(uint32_t)(alignedSize(_esize, device_minUniformBufferOffsetAlignment))}, _totalDeviceSize{_esizeDevice *
                                                                                                                                        _qty} {
    uniformBuffers.resize(_qty);
    _uniformBuffersMemory.resize(_qty);
    _cpuCopy.resize(_framebuffer_sets);
    for (size_t i = 0; i < _framebuffer_sets; i++) {
      uniformBuffers[i] = createBuffer(device, _totalDeviceSize, vk::BufferUsageFlagBits::eUniformBuffer);
      _uniformBuffersMemory[i] = AllocateBufferOnDevice(
          device, physicalDevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffers[i]);
    }
    std::cout << "Created " << _framebuffer_sets << " sets of " << _qty << " packed uniforms of size : " << _esize << " - " << _totalSize
              << " onGPU:" << _totalDeviceSize << std::endl;
  };

  T& operator[](int i) { return _cpuCopy[i]; };

  ~PackedUniform() {
    for (size_t i = 0; i < _framebuffer_sets; i++) {
      _logicalDevice.destroyBuffer(uniformBuffers[i]);
      _logicalDevice.freeMemory(_uniformBuffersMemory[i]);
    }
  }

  void sendToGpu(uint32_t currentImage) {
    void* data = _logicalDevice.mapMemory(_uniformBuffersMemory[currentImage], 0, VK_WHOLE_SIZE);
    if (_esizeDevice == _esize) {
      memcpy(data, _cpuCopy.data(), _totalSize);
    } else {
      for (uint32_t i = 0; i < _qty; ++i) {
        void* ptr = static_cast<void*>( static_cast<char*>(data) + (i * _esizeDevice));
        memcpy(ptr, &(_cpuCopy[i]), _esize);
      }
    }
    _logicalDevice.unmapMemory(_uniformBuffersMemory[currentImage]);
  };

  void sendToGpu() {
    for (size_t i = 0; i < _framebuffer_sets; i++) {
      sendToGpu(i);
    }
  }

  std::vector<vk::Buffer> uniformBuffers;

private:
  const vk::Device& _logicalDevice;
  const uint32_t _framebuffer_sets;
  const uint32_t _qty;
  const size_t _esize;
  const size_t _totalSize;
  const uint32_t _esizeDevice;
  const uint32_t _totalDeviceSize;

  std::vector<vk::DeviceMemory> _uniformBuffersMemory;
  std::vector<T> _cpuCopy;
};

struct DescriptorSetLayout {
  vk::DescriptorSetLayout descriptorSetLayout;

  DescriptorSetLayout(const vk::Device& device, const std::vector<vk::DescriptorSetLayoutBinding>& bindings);

  ~DescriptorSetLayout();

private:
  const vk::Device& _logicalDevice;
};

struct DescriptorPool {
  vk::DescriptorPool descriptorPool;

  DescriptorPool(const vk::Device& device, uint32_t frameCount, uint32_t descriptorCount);

  ~DescriptorPool();

private:
  const vk::Device& _logicalDevice;
};

struct CmdBuffers {
  struct RenderableToken {
    const VertexBuffer& vbuf;
    const uint32_t vcount;
    const std::function<void(const vk::CommandBuffer&)> descriptorSetFunc;
  };

  struct commandBufferCollection {
    vk::CommandBuffer commandBuffer;
    std::set<const VertexBuffer*> referencedVB;
    size_t hashedState = 0;
    vk::Fence* fence = nullptr;

    void Reset(const vk::Device& device);
  };

  std::vector<commandBufferCollection> commandBuffers;

  //  std::vector<size_t> commandBufferStates;
  CmdBuffers(const vk::Device& device, const vk::CommandPool& pool, size_t amount);

  void Record(const vk::Device& device, const vk::RenderPass& renderPass, const vk::Extent2D& swapChainExtent,
              const std::vector<vk::Framebuffer>& swapChainFramebuffers, const Pipeline& pipeline, const VertexBuffer& vbuf, uint32_t vcount,
              std::function<void(const vk::CommandBuffer&)> descriptorSetFunc, uint32_t index);

  void Record(const vk::Device& device, const vk::RenderPass& renderPass, const vk::Extent2D& swapChainExtent,
              const std::vector<vk::Framebuffer>& swapChainFramebuffers, const Pipeline& pipeline, std::vector<RenderableToken> tokens,
              uint32_t index);

  // wipe any commandlist that includes this VertexBuffer.
  static void invalidate(const VertexBuffer* vbuf);
  // wipe all cmdlists
  static void invalidate();
  ~CmdBuffers();

protected:
  void RecordCommands(const VertexBuffer& vbuf, uint32_t count, const vk::CommandBuffer& cmdBuffer, const vk::PipelineLayout& pipelineLayout,
                      std::function<void(const vk::CommandBuffer&)> descriptorSetFunc);

  static std::set<CmdBuffers*> _all;
  const vk::Device& _logicalDevice;
};

struct OneOffCmdBuffer {
  OneOffCmdBuffer() = delete;

  vk::CommandBuffer commandBuffer;

  OneOffCmdBuffer(const vk::Device& device, const vk::CommandPool& pool);

  void submit(vk::Queue& queue);

  ~OneOffCmdBuffer();

private:
  const vk::Device& _logicalDevice;
  vk::Fence _fence;
  vk::CommandPool _pool;
};

struct TextureImage {
  TextureImage(const vk::Device& device, const vk::PhysicalDevice& pdevice, const vk::CommandPool& pool, vk::Queue& queue);

  ~TextureImage();

  vk::ImageView _imageView;
  vk::Sampler _imageSampler;

private:
  vk::CommandPool _pool;
  const vk::Device& _logicalDevice;
  const vk::PhysicalDevice& _physicalDevice;
  vk::Queue& _queue;

  vk::Buffer _imageBuf;
  vk::DeviceMemory _imageMemory;
  vk::Image _image;

  void createImage(const vk::PhysicalDevice& pdevice, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
                   vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory);

  void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

  void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
};

void drawFrameInternal(uint32_t imageIndex, const vk::Device& device, const vk::Queue& graphicsQueue, const vk::Queue& presentQueue,
                       const vk::SwapchainKHR& swapChain, CmdBuffers::commandBufferCollection& commandBuffer, SyncObjects& sync);
