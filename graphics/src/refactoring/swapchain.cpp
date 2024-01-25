#include "swapchain.h"

Swapchain::Swapchain(VkInstance instance, Device* device,VkSurfaceKHR surface,
  SDL_Window* window,VmaAllocator allocator
) : _instance(instance), _device(device), _surface(surface), _window(window), _allocator(allocator){}

Swapchain::~Swapchain() {
  vkDestroyRenderPass(_device->_logical, _renderpass, nullptr);

  for (auto framebuffer : _framebuffers) {
    vkDestroyFramebuffer(_device->_logical, framebuffer, nullptr);
  }

  for (auto image_view : swapchain_image_views) {
    vkDestroyImageView(_device->_logical, image_view, nullptr);
  }

  delete _depth_image;

  vkDestroySwapchainKHR(_device->_logical, _swapchain, nullptr);
}

void Swapchain::create_default() {
  query_swapchain_details();

  VkSurfaceFormatKHR format = choose_surface_format();
  VkPresentModeKHR present_mode = choose_present_mode();
  VkExtent2D extent = choose_extent();

  // store details
  swapchain_image_format = format.format;
  swapchain_extent = extent;

  image_count = details.capabilities.minImageCount + 1;

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
    _device->_graphics_index.value(),
    _device->_present_index.value()
  };

  if (_device->_graphics_index.value() != _device->_present_index.value()) {
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

  VK_CHECK(vkCreateSwapchainKHR(_device->_logical, &swapchain_info, nullptr, &_swapchain));

  // retrieve swapchain images
  vkGetSwapchainImagesKHR(_device->_logical, _swapchain, &image_count, nullptr);
  swapchain_images.resize(image_count);
  vkGetSwapchainImagesKHR(_device->_logical, _swapchain, &image_count, swapchain_images.data());

  create_image_views();
}

void Swapchain::init_default_renderpass() {
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

  VkAttachmentDescription depth_attachment = {};
  depth_attachment.flags = 0;
  depth_attachment.format = _depth_image->_format;
  depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_ref = {};
  depth_attachment_ref.attachment = 1;
  depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
  subpass.pDepthStencilAttachment = &depth_attachment_ref;

  VkAttachmentDescription attachments[2] = { color_attachment,depth_attachment };

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkSubpassDependency depth_dependency = {};
  depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  depth_dependency.dstSubpass = 0;
  depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  depth_dependency.srcAccessMask = 0;
  depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkSubpassDependency dependencies[2] = { dependency, depth_dependency };

  VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//connect the color attachment to the info
	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = &attachments[0];
	//connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 2;
  render_pass_info.pDependencies = &dependencies[0];

	VK_CHECK(vkCreateRenderPass(_device->_logical, &render_pass_info, nullptr, &_renderpass));
}

void Swapchain::init_framebuffers() {
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

    VkImageView attachments[2];
		attachments[0] = swapchain_image_views[i];
    attachments[1] = _depth_image->_image_view;

    fb_info.pAttachments = attachments;
    fb_info.attachmentCount = 2;

		if (vkCreateFramebuffer(_device->_logical, &fb_info, nullptr, &_framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer");
    }
	}
}

void Swapchain::init_depth_image(VkExtent2D _window_extent) {
  //depth image size will match window
  _depth_image = new Image(_allocator, _device->_logical);
  _depth_image->create_depth_image(_window_extent);
}

void Swapchain::query_swapchain_details() {
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->_physical, _surface, &details.capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(_device->_physical, _surface, &format_count, nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(_device->_physical, _surface, &format_count, details.formats.data());
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(_device->_physical, _surface, &present_mode_count, nullptr);

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(_device->_physical, _surface, &present_mode_count, details.present_modes.data());
  }
}


VkSurfaceFormatKHR Swapchain::choose_surface_format() {
  for (const auto& available_format : details.formats) {
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && 
    available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
    }
  }

  return details.formats[0];
}

VkPresentModeKHR Swapchain::choose_present_mode() {
  for (const auto& present_mode : details.present_modes) {
    if (present_mode == VK_PRESENT_MODE_FIFO_KHR) {
      return present_mode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::choose_extent() {
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


void Swapchain::create_image_views() {
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

    VK_CHECK(vkCreateImageView(_device->_logical, &image_info, nullptr, &swapchain_image_views[i]));
  }
}

VkRenderPassBeginInfo Swapchain::renderpass_begin_info(VkExtent2D _window_extent, uint32_t swapchain_image_index) {
  VkRenderPassBeginInfo renderpass_info{};
  renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderpass_info.pNext = nullptr;

  renderpass_info.renderPass = _renderpass;
	renderpass_info.renderArea.offset.x = 0;
	renderpass_info.renderArea.offset.y = 0;
	renderpass_info.renderArea.extent = _window_extent;
	renderpass_info.framebuffer = _framebuffers[swapchain_image_index];

  return renderpass_info;
}