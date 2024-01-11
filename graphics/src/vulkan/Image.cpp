#include "Image.h"

namespace GRAPHICS
{


Image::Image(VmaAllocator allocator, VkDevice device, VkExtent2D _window_extent) {
  _device = device;
  _allocator = allocator;
  
  VkExtent3D draw_image_extent = {
    _window_extent.width,
    _window_extent.height,
    1
  };

  //draw format is 32 bit float
  image_format = VK_FORMAT_R16G16B16A16_SFLOAT;
  image_extent = draw_image_extent;

  VkImageUsageFlags draw_image_usages{};
  draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_STORAGE_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  VkImageCreateInfo image_info = image_create_info(image_format, draw_image_usages, draw_image_extent);
  
  // allocate the image on local gpu memory
  VmaAllocationCreateInfo image_alloc_info{};
  image_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  image_alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  vmaCreateImage(_allocator, &image_info, &image_alloc_info, &image, &allocation, nullptr);

  // build image view to use for rendering
  VkImageViewCreateInfo view_info = imageview_create_info(image_format, image, VK_IMAGE_ASPECT_COLOR_BIT);
  if (vkCreateImageView(_device, &view_info, nullptr, &image_view) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image view");
  }
}

Image::~Image() {
  vkDestroyImageView(_device, image_view, nullptr);
  vmaDestroyImage(_allocator, image, allocation);
}

VkImageCreateInfo Image::image_create_info(VkFormat format, 
  VkImageUsageFlags usage_flags, 
  VkExtent3D extent) {
  
  VkImageCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.pNext = nullptr;

  info.imageType = VK_IMAGE_TYPE_2D;

  info.format = format;
  info.extent = extent;

  info.mipLevels = 1;
  info.arrayLayers = 1;

  //for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
  info.samples = VK_SAMPLE_COUNT_1_BIT;

  //optimal tiling, which means the image is stored on the best gpu format
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.usage = usage_flags;

  return info;
}

VkImageViewCreateInfo Image::imageview_create_info(VkFormat format, 
  VkImage image, 
  VkImageAspectFlags aspect_flags) {
  
  // build a image-view for the depth image to use for rendering
  VkImageViewCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  info.pNext = nullptr;

  info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  info.image = image;
  info.format = format;
  info.subresourceRange.baseMipLevel = 0;
  info.subresourceRange.levelCount = 1;
  info.subresourceRange.baseArrayLayer = 0;
  info.subresourceRange.layerCount = 1;
  info.subresourceRange.aspectMask = aspect_flags;

  return info;
}

} // namespace GRAPHICS

