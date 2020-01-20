//
// Created by Sam Serrels on 30/11/2019.
//
#include "../../platform/platform_glfw.h"

#include <cmath>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "vulkan_data.h"
#include "vulkan_internals.h"
//#include <vulkan/vulkan.hpp>

// VkInfo* info;

const bool enableValidationLayers = true;

// const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
};
struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

static std::string BytesToString(unsigned long long byteCount) {
  std::string suf[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB"}; // Longs run out around EB
  if (byteCount == 0)
    return "0" + suf[0];
  int place = (int)(floor(log(byteCount) / log(1024)));
  double a = (byteCount / pow(1024, place));
  const int precisionVal = 3;
  const std::string trimmedString = std::to_string(a).substr(0, std::to_string(a).find(".") + precisionVal + 1);
  return trimmedString + suf[place];
}

void probe() {
  VkResult result;
  VkInstanceCreateInfo info = {};
  uint32_t instance_layer_count;

  result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
  std::cout << instance_layer_count << " layers found!\n";
  if (instance_layer_count > 0) {
    std::unique_ptr<VkLayerProperties[]> instance_layers(new VkLayerProperties[instance_layer_count]);
    result = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.get());
    for (uint32_t i = 0; i < instance_layer_count; ++i) {
      std::cout << instance_layers[i].layerName << "\n";
    }
  }
}

vk::Format findSupportedFormat(const vk::PhysicalDevice& device, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                               vk::FormatFeatureFlags features) {
  for (const auto& format : candidates) {
    vk::FormatProperties props = device.getFormatProperties(format);
    if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {
  SwapChainSupportDetails details;
  details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
  details.formats = device.getSurfaceFormatsKHR(surface);
  details.presentModes = device.getSurfacePresentModesKHR(surface);
  return details;
}

SwapChainSupportDetails querySwapChainSupport(const ContextInfo::PhyDevSurfKHR& pds) { return querySwapChainSupport(pds.device, pds.surface); }

const char* checkValidationLayerSupport() {
  const std::vector<const char*> coudHave{"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_standard_validation"};
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char* layerName : coudHave) {
    bool layerFound = false;
    for (const auto& layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        return layerName;
      }
    }
  }
  return "";
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}

QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {
  QueueFamilyIndices indices;
  std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
  int i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphicsFamily = i;
    }

    VkBool32 presentSupport = device.getSurfaceSupportKHR(i, surface);
    if (presentSupport) {
      indices.presentFamily = i;
    }

    if (indices.isComplete()) {
      break;
    }

    i++;
  }

  return indices;
}
QueueFamilyIndices findQueueFamilies(const struct ContextInfo::PhyDevSurfKHR pdsk) { return findQueueFamilies(pdsk.device, pdsk.surface); }
bool checkDeviceExtensionSupport(const vk::PhysicalDevice& pdevice) {

  std::vector<vk::ExtensionProperties> availableExtensions = pdevice.enumerateDeviceExtensionProperties();
  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

vk::PhysicalDevice pickPhysicalDevice(const vk::Instance& instance, const vk::SurfaceKHR& surface) {
  std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

  if (devices.empty()) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<vk::PhysicalDevice> compatableDevices;
  for (const auto& device : devices) {
    if (findQueueFamilies(device, surface).isComplete() && checkDeviceExtensionSupport(device)) {
      compatableDevices.push_back(device);
    }
  }

  vk::DeviceSize biggestRamAmount = 0;
  vk::PhysicalDevice BestGPU;

  std::cout << "comaptible GPU devices:"
            << "\n";

  for (const auto g : compatableDevices) {
    auto dp = g.getProperties();
    vk::DeviceSize memsize = 0;
    {
      auto memoryProps = g.getMemoryProperties();
      auto heapsPointer = memoryProps.memoryHeaps;
      auto heaps = std::vector<vk::MemoryHeap>(heapsPointer, heapsPointer + memoryProps.memoryHeapCount);
      for (const auto& heap : heaps) {
        if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
          memsize = heap.size;
          if (memsize > biggestRamAmount) {
            biggestRamAmount = memsize;
            BestGPU = g;
          }
          break;
        }
      }
    }
    std::cout << "Addr: " << &g << " Device " << dp.deviceID << " - " << dp.deviceName << " - " << dp.driverVersion
              << " vram: " << BytesToString(memsize) << "\n";
  }
  if (BestGPU == vk::PhysicalDevice(nullptr)) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
  vk::PhysicalDeviceProperties props = BestGPU.getProperties();
  device_minUniformBufferOffsetAlignment = (uint32_t)props.limits.minUniformBufferOffsetAlignment;
  device_maxDescriptorSetUniformBuffersDynamic = props.limits.maxDescriptorSetUniformBuffersDynamic;
  std::cout << "Using Device " << &BestGPU << std::endl;
  return BestGPU;
}

vk::Device createLogicalDevice(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, vk::Queue& graphicsQueue,
                               vk::Queue& presentQueue) {
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    vk::DeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures deviceFeatures = {};

  vk::DeviceCreateInfo createInfo = {};
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (enableValidationLayers) {
    // createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    // createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }
  vk::Device logicalDevice = physicalDevice.createDevice(createInfo, nullptr);
  // if ( != vk::Result::eSuccess) {
  //   throw std::runtime_error("failed to create logical device!");
  // }

  graphicsQueue = logicalDevice.getQueue(indices.graphicsFamily.value(), 0);
  presentQueue = logicalDevice.getQueue(indices.presentFamily.value(), 0);
  std::cout << "Logical Device is cool" << std::endl;
  return logicalDevice;
}

CmdPool::CmdPool(const ContextInfo::PhyDevSurfKHR& PhyDevSurf, const vk::Device& device) : _logicalDevice{device} {
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(PhyDevSurf);

  vk::CommandPoolCreateInfo poolInfo = {};
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
  poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  commandPool = device.createCommandPool(poolInfo);
}

CmdPool::~CmdPool() {
  _logicalDevice.destroyCommandPool(commandPool);
  // commandBuffers.clear();
}

SyncObjects::SyncObjects(const vk::Device& device, const size_t swapChainImagesCount) : _logicalDevice{device} {
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  imagesInFlight.resize(swapChainImagesCount);

  vk::SemaphoreCreateInfo semaphoreInfo;
  vk::FenceCreateInfo fenceInfo;
  fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    imageAvailableSemaphores[i] = device.createSemaphore(semaphoreInfo);
    renderFinishedSemaphores[i] = device.createSemaphore(semaphoreInfo);
    inFlightFences[i] = device.createFence(fenceInfo);
  }
}

SyncObjects::~SyncObjects() {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    _logicalDevice.destroySemaphore(renderFinishedSemaphores[i]);
    _logicalDevice.destroySemaphore(imageAvailableSemaphores[i]);
    _logicalDevice.destroyFence(inFlightFences[i]);
  }
  imageAvailableSemaphores.clear();
  renderFinishedSemaphores.clear();
  inFlightFences.clear();
  imagesInFlight.clear();
}

vk::Instance CreateInstance(vk::DebugUtilsMessengerEXT* debugMessenger = nullptr) {
  vk::ApplicationInfo appInfo;
  appInfo.pApplicationName = "OpenLRR";
  appInfo.pEngineName = "OpenLRR";
  appInfo.apiVersion = VK_API_VERSION_1_1;
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

  vk::InstanceCreateInfo instanceCreateInfo;
  instanceCreateInfo.pApplicationInfo = &appInfo;

  // Enable extensions
  std::vector<const char*> extensions;
  {
    uint32_t glfwExtensionCount = 0;
    const auto glfwExtensions = platform::GetRequiredVKInstanceExtensions(&glfwExtensionCount);
    extensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
  }

  const auto validationLayerName = checkValidationLayerSupport();
  if (enableValidationLayers) {
    if (validationLayerName[0] == '\0') {
      probe();
      throw std::runtime_error("validation layers requested, but not available!");
    } else {
      instanceCreateInfo.enabledLayerCount = 1;
      instanceCreateInfo.ppEnabledLayerNames = &(validationLayerName);
    }
  } else {
    instanceCreateInfo.enabledLayerCount = 0;
  }

  instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

  vk::Instance instance;
  try {
    instance = vk::createInstance(instanceCreateInfo, nullptr);
  } catch (vk::SystemError err) {
    throw std::runtime_error("failed to create instance!");
  }
  // DEbug validation
  if (enableValidationLayers) {

    auto createInfo =
        vk::DebugUtilsMessengerCreateInfoEXT(vk::DebugUtilsMessengerCreateFlagsEXT(),
                                             vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                 vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                                             vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                                             debugCallback, nullptr);
    // instance.createDebugUtilsMessengerEXT(createInfo); //broken?

    PFN_vkCreateDebugUtilsMessengerEXT func =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));

    if (debugMessenger != nullptr && func != nullptr &&
        func(static_cast<VkInstance>(instance), reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr,
             reinterpret_cast<VkDebugUtilsMessengerEXT*>(debugMessenger)) == VK_SUCCESS) {
      std::cout << "Created DebugUtils Messenger" << std::endl;
    } else {
      throw std::runtime_error("VK_ERROR_EXTENSION_NOT_PRESENT or nullptr!");
    }
  }

  std::cout << "Enabled Layers:" << std::endl;
  for (uint32_t i = 0; i < instanceCreateInfo.enabledLayerCount; ++i) {
    std::cout << instanceCreateInfo.ppEnabledLayerNames[i] << "\n";
  }
  std::cout << "Enabled Extensions:" << std::endl;
  for (uint32_t i = 0; i < instanceCreateInfo.enabledExtensionCount; ++i) {
    std::cout << instanceCreateInfo.ppEnabledExtensionNames[i] << "\n";
  }
  return instance;
}

ContextInfo::ContextInfo()
    : instance{CreateInstance(&debugMessenger)}, surface{CreateVKWindowSurface(instance)}, physicalDevice{pickPhysicalDevice(instance, surface)},
      deviceKHR{physicalDevice, surface}, device{createLogicalDevice(physicalDevice, surface, graphicsQueue, presentQueue)} {

  std::cout << "Context COOL" << std::endl;
}

ContextInfo::~ContextInfo() {
  std::cout << "Goodbye from Context" << std::endl;
  device.destroy();
  device = nullptr;
  graphicsQueue = nullptr;
  presentQueue = nullptr;
  instance.destroySurfaceKHR(surface);
  surface = nullptr;

  if (debugMessenger != vk::DebugUtilsMessengerEXT(nullptr)) {
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) {
      func(static_cast<VkInstance>(instance), static_cast<VkDebugUtilsMessengerEXT>(debugMessenger), nullptr);
    }
  }
  instance.destroy();
}

SwapChainInfo::SwapChainInfo(const ContextInfo::PhyDevSurfKHR& pds, const vk::Device& logicalDevice, const vk::RenderPass& renderPass)
    : _logicalDevice{logicalDevice} {
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pds);

  vk::SurfaceFormatKHR format = getImageFormat(pds);

  // Present Mode
  vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
  if (!ENABLE_VSYNC) {
    for (const auto& availablePresentMode : swapChainSupport.presentModes) {
      if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
        presentMode = availablePresentMode;
        break;
      }
    }
  }

  // Extent
  VkExtent2D extent;
  {
    const auto cap = swapChainSupport.capabilities;
    if (cap.currentExtent.width != UINT32_MAX) {
      extent = cap.currentExtent;
    } else {
      int width, height;
      platform::GetFramebufferSize(&width, &height);
      VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
      actualExtent.width = std::max(cap.minImageExtent.width, std::min(cap.maxImageExtent.width, actualExtent.width));
      actualExtent.height = std::max(cap.minImageExtent.height, std::min(cap.maxImageExtent.height, actualExtent.height));
      extent = actualExtent;
    }
  }
  swapChainExtent = extent;

  // Imagecount
  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  // depth

  {
    vk::Format depthFormat = getDepthFormat(pds.device);
    createImage(pds.device, logicalDevice, swapChainExtent.width, swapChainExtent.height, depthFormat, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
    depthImageView = createImageView(logicalDevice, depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
  }

  // Actual swapchain
  {
    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = pds.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    QueueFamilyIndices indices = findQueueFamilies(pds);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
      createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
      createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    swapChain = logicalDevice.createSwapchainKHR(createInfo, nullptr);
    swapChainImages = logicalDevice.getSwapchainImagesKHR(swapChain);
  }

  // imageviews
  {
    swapChainImageViews.resize(swapChainImages.size());
    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
      swapChainImageViews[i] = createImageView(logicalDevice, swapChainImages[i], format.format, vk::ImageAspectFlagBits::eColor);
    }
  }

  // Framebuffers
  {
    swapChainFramebuffers.clear();
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      std::array<vk::ImageView, 2> attachments = {swapChainImageViews[i], depthImageView};

      vk::FramebufferCreateInfo framebufferInfo;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      swapChainFramebuffers[i] = _logicalDevice.createFramebuffer(framebufferInfo);
    }
    std::cout << swapChainImageViews.size() << " framebuffers created" << std::endl;
  }

  std::cout << "Swap Chain is created: " << swapChainExtent.height << "x" << swapChainExtent.width << ", Images: " << swapChainImages.size()
            << std::endl;
}

SwapChainInfo::~SwapChainInfo() {

  if (!swapChainFramebuffers.empty()) {
    for (auto framebuffer : swapChainFramebuffers) {
      _logicalDevice.destroyFramebuffer(framebuffer);
    }
    swapChainFramebuffers.clear();
  }
  for (auto imageView : swapChainImageViews) {
    _logicalDevice.destroyImageView(imageView);
  }
  _logicalDevice.destroyImageView(depthImageView);
  _logicalDevice.destroyImage(depthImage);
  _logicalDevice.freeMemory(depthImageMemory);
  swapChainImages.clear();
  _logicalDevice.destroySwapchainKHR(swapChain);
}

vk::SurfaceFormatKHR SwapChainInfo::getImageFormat(const ContextInfo::PhyDevSurfKHR& pds) {
  vk::SurfaceFormatKHR surfaceFormat;
  auto formats = pds.device.getSurfaceFormatsKHR(pds.surface);
  for (const auto& availableFormat : formats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      surfaceFormat = availableFormat;
      break;
    }
  }
  if (surfaceFormat.format == vk::Format::eUndefined) {
    surfaceFormat = formats[0];
  }

  return surfaceFormat;
}

vk::Format SwapChainInfo::getDepthFormat(const vk::PhysicalDevice& physicalDevice) {
  return findSupportedFormat(physicalDevice, {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                             vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::UniqueRenderPass createRenderPass(const vk::Device& device, const vk::Format& swapChainImageFormat, const vk::Format& swapChainDepthFormat) {

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

  vk::AttachmentDescription depthAttachment = {};
  depthAttachment.format = swapChainDepthFormat;
  depthAttachment.samples = vk::SampleCountFlagBits::e1;
  depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
  depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
  depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
  depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

  vk::AttachmentReference depthAttachmentRef = {};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

  vk::SubpassDescription subpass;
  subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  vk::SubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.srcAccessMask = vk::AccessFlags();
  dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

  std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
  vk::RenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  return device.createRenderPassUnique(renderPassInfo);
}

void vk_DestoryRenderPass(std::unique_ptr<vk::RenderPass> rp, const vk::Device& device) { device.destroyRenderPass(*rp); }
