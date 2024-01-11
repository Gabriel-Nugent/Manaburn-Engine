#pragma once

#include <vulkan/vulkan.h>

namespace vkutil
{

void vkutil::copy_image_to_image(VkCommandBuffer cmd, VkImage src, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size) {
  VkImageBlit blit_region{};
  
  blit_region.srcOffsets[1].x = src_size.width;
  blit_region.srcOffsets[1].x = src_size.height;
  blit_region.srcOffsets[1].z = 1;

  blit_region.dstOffsets[1].x = dst_size.width;
	blit_region.dstOffsets[1].y = dst_size.height;
	blit_region.dstOffsets[1].z = 1;

	blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.srcSubresource.baseArrayLayer = 0;
	blit_region.srcSubresource.layerCount = 1;
	blit_region.srcSubresource.mipLevel = 0;

	blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.dstSubresource.baseArrayLayer = 0;
	blit_region.dstSubresource.layerCount = 1;
	blit_region.dstSubresource.mipLevel = 0;

  vkCmdBlitImage(
    cmd, 
    src, 
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
    destination, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    1, 
    &blit_region, 
    VK_FILTER_LINEAR
  );
}

} // namespace vkutil
