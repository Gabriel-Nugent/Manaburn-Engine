#pragma once

#include <vulkan/vulkan.h>

#include "../../external_src/vk_mem_alloc.h"
#include "../vulkan_util/vk_types.h"

class Image
{
public:
  Image(VmaAllocator allocator, VkDevice device);
  ~Image();

  void create_depth_image(VkExtent2D _window_extent);

  VkImage       _image = VK_NULL_HANDLE;
  VkFormat      _format;
  VkImageView   _image_view = VK_NULL_HANDLE;
  VmaAllocation _allocation;

private:
  VkDevice      _device;
  VmaAllocator  _allocator;

  static VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent);
  static VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags);
};

