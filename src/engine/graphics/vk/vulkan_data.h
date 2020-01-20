#pragma once
#include <vulkan/vulkan.hpp>

uint32_t findMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags& properties, const vk::PhysicalDevice& pdevice);
vk::ImageView createImageView(const vk::Device& device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);
void createImage(const vk::PhysicalDevice& pdevice, const vk::Device& lDevice, uint32_t width, uint32_t height, vk::Format format,
                 vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image,
                 vk::DeviceMemory& imageMemory);
