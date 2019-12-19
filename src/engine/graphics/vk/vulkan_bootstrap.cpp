//
// Created by Sam Serrels on 30/11/2019.
//
#define NOMINMAX
#include "../../platform/platform_glfw.h"
#include "vulkan_internals.h"
#include <cmath>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

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
  int place = (floor(log(byteCount) / log(1024)));
  double a = (byteCount / pow(1024, place));
  const int precisionVal = 3;
  const std::string trimmedString = std::to_string(a).substr(0, std::to_string(a).find(".") + precisionVal + 1);
  return trimmedString + suf[place];
}

void probe() {
  VkInstance pinstance;
  VkResult result;
  VkInstanceCreateInfo info = {};
  uint32_t instance_layer_count;

  result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
  std::cout << instance_layer_count << " layers found!\n";
  if (instance_layer_count > 0) {
    std::unique_ptr<VkLayerProperties[]> instance_layers(new VkLayerProperties[instance_layer_count]);
    result = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.get());
    for (int i = 0; i < instance_layer_count; ++i) {
      std::cout << instance_layers[i].layerName << "\n";
    }
  }
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

QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice device, const vk::SurfaceKHR& surface) {
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
bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

VkPhysicalDevice pickPhysicalDevice(const vk::Instance& instance, const VkSurfaceKHR& surface) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  std::vector<VkPhysicalDevice> compatableDevices;
  for (const auto& device : devices) {
    if (findQueueFamilies(device, surface).isComplete() && checkDeviceExtensionSupport(device)) {
      compatableDevices.push_back(device);
    }
  }

  vk::DeviceSize biggestRamAmount = 0;
  VkPhysicalDevice BestGPU = VK_NULL_HANDLE;

  std::cout << "comaptible GPU devices:"
            << "\n";

  for (const auto g : compatableDevices) {
    VkPhysicalDeviceProperties dp;
    vkGetPhysicalDeviceProperties(g, &dp);
    vk::DeviceSize memsize = 0;
    {
      auto memoryProps = VkPhysicalDeviceMemoryProperties{};
      vkGetPhysicalDeviceMemoryProperties(g, &memoryProps);
      auto heapsPointer = memoryProps.memoryHeaps;
      auto heaps = std::vector<VkMemoryHeap>(heapsPointer, heapsPointer + memoryProps.memoryHeapCount);
      for (const auto& heap : heaps) {
        if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
          memsize = heap.size;
          if (memsize > biggestRamAmount) {
            biggestRamAmount = memsize;
            BestGPU = g;
          }
          break;
        }
      }
    }
    std::cout << "Addr: " << g << " Device " << dp.deviceID << " - " << dp.deviceName << " - " << dp.driverVersion
              << " vram: " << BytesToString(memsize) << "\n";
  }
  if (BestGPU == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
  std::cout << "Using Device " << BestGPU << std::endl;
  return BestGPU;
}

vk::Device createLogicalDevice(const vk::PhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, vk::Queue& graphicsQueue,
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
  vkDestroyCommandPool(_logicalDevice, commandPool, nullptr);
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
    vkDestroySemaphore(_logicalDevice, renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(_logicalDevice, imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(_logicalDevice, inFlightFences[i], nullptr);
  }
  imageAvailableSemaphores.clear();
  renderFinishedSemaphores.clear();
  inFlightFences.clear();
  imagesInFlight.clear();
}

vk::Instance CreateInstance(VkDebugUtilsMessengerEXT* debugMessenger = nullptr) {
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
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (debugMessenger != nullptr && func != nullptr &&
        func(instance, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr, debugMessenger) == VK_SUCCESS) {
      std::cout << "Created DebugUtils Messenger" << std::endl;
    } else {
      throw std::runtime_error("VK_ERROR_EXTENSION_NOT_PRESENT or nullptr!");
    }
  }

  std::cout << "Enabled Layers:" << std::endl;
  for (int i = 0; i < instanceCreateInfo.enabledLayerCount; ++i) {
    std::cout << instanceCreateInfo.ppEnabledLayerNames[i] << "\n";
  }
  std::cout << "Enabled Extensions:" << std::endl;
  for (int i = 0; i < instanceCreateInfo.enabledExtensionCount; ++i) {
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
  vkDestroyDevice(device, nullptr);
  device = nullptr;
  graphicsQueue = nullptr;
  presentQueue = nullptr;
  vkDestroySurfaceKHR(instance, surface, nullptr);
  if (debugMessenger != nullptr) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
      func(instance, debugMessenger, nullptr);
    }
  }
  vkDestroyInstance(instance, nullptr);
}

VkSwapchainKHR createSwapChain(const ContextInfo::PhyDevSurfKHR& pds, const vk::Device& logicalDevice, std::vector<vk::Image>& swapChainImages,
                               vk::Format& swapChainImageFormat, vk::Extent2D& swapChainExtent) {

  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pds);

  vk::SurfaceFormatKHR surfaceFormat;
  {
    for (const auto& availableFormat : swapChainSupport.formats) {
      if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
        surfaceFormat = availableFormat;
        break;
      }
    }
    if (surfaceFormat.format == vk::Format::eUndefined) {
      surfaceFormat = swapChainSupport.formats[0];
    }
  }

  vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
  if (!ENABLE_VSYNC) {
    for (const auto& availablePresentMode : swapChainSupport.presentModes) {
      if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
        presentMode = availablePresentMode;
        break;
      }
    }
  }

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

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo;
  createInfo.surface = pds.surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
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
  // createInfo.oldSwapchain = VK_NULL_HANDLE;
  vk::SwapchainKHR swapChain = logicalDevice.createSwapchainKHR(createInfo, nullptr);

  swapChainImages = logicalDevice.getSwapchainImagesKHR(swapChain);
  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
  std::cout << "Swap Chain is cool: " << swapChainExtent.height << "x" << swapChainExtent.width << std::endl;

  return swapChain;
}

std::vector<vk::ImageView> createImageViews(const std::vector<vk::Image>& swapChainImages, const vk::Format& swapChainImageFormat,
                                            vk::Device logicalDevice) {
  std::vector<vk::ImageView> swapChainImageViews;
  swapChainImageViews.resize(swapChainImages.size());

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    vk::ImageViewCreateInfo createInfo;
    createInfo.image = swapChainImages[i];
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format = swapChainImageFormat;
    createInfo.components.r = vk::ComponentSwizzle::eIdentity;
    createInfo.components.g = vk::ComponentSwizzle::eIdentity;
    createInfo.components.b = vk::ComponentSwizzle::eIdentity;
    createInfo.components.a = vk::ComponentSwizzle::eIdentity;
    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    swapChainImageViews[i] = logicalDevice.createImageView(createInfo);
  }
  return swapChainImageViews;
}

SwapChainInfo::SwapChainInfo(const ContextInfo::PhyDevSurfKHR& pds, const vk::Device& logicalDevice)
    : swapChain{createSwapChain(pds, logicalDevice, swapChainImages, swapChainImageFormat, swapChainExtent)},
      swapChainImageViews{createImageViews(swapChainImages, swapChainImageFormat, logicalDevice)}, _logicalDevice{logicalDevice} {
  std::cout << "SWAP CHAIN COOL" << std::endl;
}

SwapChainInfo::~SwapChainInfo() {

  if (!swapChainFramebuffers.empty()) {
    for (auto framebuffer : swapChainFramebuffers) {
      vkDestroyFramebuffer(_logicalDevice, framebuffer, nullptr);
    }
  }
  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(_logicalDevice, imageView, nullptr);
  }
  swapChainImages.clear();
  vkDestroySwapchainKHR(_logicalDevice, swapChain, nullptr);
}
