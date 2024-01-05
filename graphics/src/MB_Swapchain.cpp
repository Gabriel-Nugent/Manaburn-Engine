#include "MB_Swapchain.h"

namespace GRAPHICS
{

MB_Swapchain::MB_Swapchain(
  VkInstance instance, 
  MB_Device* device, 
  VkSurfaceKHR surface,
  SDL_Window* window)
: _instance(instance), mb_device(device), _surface(surface), _window(window) {
  _gpu = mb_device->get_gpu();
}

MB_Swapchain::~MB_Swapchain() {
  vkDestroyRenderPass(mb_device->get_device(), _renderpass, nullptr);

  for (auto framebuffer : _framebuffers) {
    vkDestroyFramebuffer(mb_device->get_device(), framebuffer, nullptr);
  }

  for (auto image_view : swapchain_image_views) {
    vkDestroyImageView(mb_device->get_device(), image_view, nullptr);
  }

  vkDestroySwapchainKHR(mb_device->get_device(), _swapchain, nullptr);
}

void MB_Swapchain::create_default() {
  query_swapchain_details();

  VkSurfaceFormatKHR format = choose_surface_format();
  VkPresentModeKHR present_mode = choose_present_mode();
  VkExtent2D extent = choose_extent();

  // store details
  swapchain_image_format = format.format;
  swapchain_extent = extent;

  uint32_t image_count = details.capabilities.minImageCount + 1;

  if(details.capabilities.maxImageCount > 0 && 
  image_count > details.capabilities.maxImageCount) {
    image_count = details.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapchain_info{};
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface = _surface;
  swapchain_info.minImageCount = image_count;
  swapchain_info.imageFormat = format.format;
  swapchain_info.imageColorSpace = format.colorSpace;
  swapchain_info.imageExtent = extent;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queue_family_indices[] = {
    mb_device->get_graphics_index(),
    mb_device->get_present_index()
  };

  if (mb_device->get_graphics_index() != mb_device->get_present_index()) {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_info.queueFamilyIndexCount = 2;
    swapchain_info.pQueueFamilyIndices = queue_family_indices;
  }
  else {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info.queueFamilyIndexCount = 0;
    swapchain_info.pQueueFamilyIndices = nullptr;
  }

  swapchain_info.preTransform = details.capabilities.currentTransform;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode = present_mode;
  swapchain_info.clipped = VK_TRUE;
  swapchain_info.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(
    mb_device->get_device(), 
    &swapchain_info, 
    nullptr, 
    &_swapchain
  ) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  // retrieve swapchain images
  vkGetSwapchainImagesKHR(mb_device->get_device(), _swapchain, &image_count, nullptr);
  swapchain_images.resize(image_count);
  vkGetSwapchainImagesKHR(mb_device->get_device(), _swapchain, &image_count, swapchain_images.data());

  create_image_views();
}

void MB_Swapchain::create() {

}

void MB_Swapchain::recreate() {

}

void MB_Swapchain::init_default_renderpass() {
  VkAttachmentDescription color_attachment = {};
	//the attachment will have the format needed by the swapchain
	color_attachment.format = swapchain_image_format;
	//1 sample, we won't be doing MSAA
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// we Clear when this attachment is loaded
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// we keep the attachment stored when the renderpass ends
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//we don't care about stencil
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//we don't know or care about the starting layout of the attachment
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//after the renderpass ends, the image has to be on a layout ready for display
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref = {};
	//attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

  VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//connect the color attachment to the info
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	//connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	if (vkCreateRenderPass(mb_device->get_device(), &render_pass_info, nullptr, &_renderpass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create renderpass!");
  }
}

void MB_Swapchain::init_framebuffers() {
  //create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = nullptr;

	fb_info.renderPass = _renderpass;
	fb_info.attachmentCount = 1;
	fb_info.width = swapchain_extent.width;
	fb_info.height = swapchain_extent.height;
	fb_info.layers = 1;

	//grab how many images we have in the swapchain
	const uint32_t swapchain_imagecount = static_cast<uint32_t>(swapchain_images.size());
	_framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	//create framebuffers for each of the swapchain image views
	for (uint32_t i = 0; i < swapchain_imagecount; i++) {

		fb_info.pAttachments = &swapchain_image_views[i];
		if (vkCreateFramebuffer(mb_device->get_device(), &fb_info, nullptr, &_framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer");
    }
	}
}

void MB_Swapchain::query_swapchain_details() {
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_gpu, _surface, &details.capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(_gpu, _surface, &format_count, nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
      _gpu, _surface, 
      &format_count, 
      details.formats.data()
    );
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(_gpu, _surface, &present_mode_count, nullptr);

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      _gpu, 
      _surface, 
      &present_mode_count, 
      details.present_modes.data());
  }
}


VkSurfaceFormatKHR MB_Swapchain::choose_surface_format() {
  for (const auto& available_format : details.formats) {
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && 
    available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
    }
  }

  return details.formats[0];
}

VkPresentModeKHR MB_Swapchain::choose_present_mode() {
  for (const auto& present_mode : details.present_modes) {
    if (present_mode == VK_PRESENT_MODE_FIFO_KHR) {
      return present_mode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D MB_Swapchain::choose_extent() {
  if (details.capabilities.currentExtent.width != 
  std::numeric_limits<uint32_t>::max()) {
    return details.capabilities.currentExtent;
  }
  else {
    int width, height;
    SDL_Vulkan_GetDrawableSize(_window, &width, &height);

    VkExtent2D extent = {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height)
    };

    extent.width = std::clamp(
      extent.width, 
      details.capabilities.minImageExtent.width,
      details.capabilities.maxImageExtent.width
    );

    extent.height = std::clamp(
      extent.height, 
      details.capabilities.minImageExtent.height,
      details.capabilities.maxImageExtent.height
    );

    return extent;
  }
}


void MB_Swapchain::create_image_views() {
  swapchain_image_views.resize(swapchain_images.size());

  for (size_t i = 0; i < swapchain_images.size(); i++) {
    VkImageViewCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_info.image = swapchain_images[i];
    image_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_info.format = swapchain_image_format;
    image_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_info.subresourceRange.baseMipLevel = 0;
    image_info.subresourceRange.levelCount = 1;
    image_info.subresourceRange.baseArrayLayer = 0;
    image_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(mb_device->get_device(), &image_info, nullptr, &swapchain_image_views[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create image views!");
    }
  }
}


} // namespace MB
