#include "Image.h"

namespace GRAPHICS
{

Image::Image(VmaAllocator allocator, VkDevice device) : _allocator(allocator), _device(device) {}

Image::~Image() {
  if (_image_view != VK_NULL_HANDLE) {
    vkDestroyImageView(_device, _image_view, nullptr);
    _image_view = VK_NULL_HANDLE;
  }
  if (_image != VK_NULL_HANDLE) {
    vmaDestroyImage(_allocator, _image, _allocation);
    _image = VK_NULL_HANDLE;
  }
}
  
void Image::create_depth_image(VkExtent2D _window_extent) {
  VkExtent3D depth_image_extent = {
    _window_extent.width,      
    _window_extent.height,
    1
  };

  _format = VK_FORMAT_D32_SFLOAT;

  VkImageCreateInfo depth_img_info = image_create_info(_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depth_image_extent);

  // allocate depth image on GPU local memory
  VmaAllocationCreateInfo depth_img_alloc_info {};
  depth_img_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  depth_img_alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  vmaCreateImage(_allocator, &depth_img_info, &depth_img_alloc_info, &_image, &_allocation, nullptr);

  // image view is used for rendering
  VkImageViewCreateInfo depth_view_info = imageview_create_info(_format, _image, VK_IMAGE_ASPECT_DEPTH_BIT);
  VK_CHECK(vkCreateImageView(_device, &depth_view_info, nullptr, &_image_view));
}
  
VkImageCreateInfo Image::image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent) {
  VkImageCreateInfo info = { };
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.pNext = nullptr;

  info.imageType = VK_IMAGE_TYPE_2D;

  info.format = format;
  info.extent = extent;

  info.mipLevels = 1;
  info.arrayLayers = 1;
  info.samples = VK_SAMPLE_COUNT_1_BIT;
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.usage = usage_flags;

  return info;
}
  
VkImageViewCreateInfo Image::imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags) {
  //build a image-view for the depth image to use for rendering
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

