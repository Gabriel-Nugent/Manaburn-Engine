#pragma once

#include "Swapchain.h"

#include <vulkan/vulkan.h>
#include "../external_src/vk_mem_alloc.h"

namespace GRAPHICS
{

class Image
{
public:
  Image(VmaAllocator _allocator, VkDevice _device, VkExtent2D _window_extent);
  ~Image();

  VkImage image;
  VkImageView image_view;
  VmaAllocation allocation;
  VkExtent3D image_extent;
  VkFormat image_format;

  VkImageCreateInfo image_create_info(
    VkFormat format, 
    VkImageUsageFlags usage_flags, 
    VkExtent3D extent
  );

  VkImageViewCreateInfo imageview_create_info(
    VkFormat format, 
    VkImage image, 
    VkImageAspectFlags aspect_flags
  );

private:
  VkDevice _device;
  VmaAllocator _allocator;
};

} // namespace GRAPHICS