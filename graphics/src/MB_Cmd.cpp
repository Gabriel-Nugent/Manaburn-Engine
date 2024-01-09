#include "MB_Cmd.h"

namespace GRAPHICS
{

MB_Cmd::MB_Cmd(MB_Device* device) {
  _device = device->get_device();
  _graphics_queue = device->get_graphics_queue();
  _graphics_queue_family = device->get_graphics_index();
}

MB_Cmd::~MB_Cmd() {
  for (int i = 0; i < FRAME_OVERLAP; i++) {
    vkDestroyCommandPool(_device, _frames[i]._command_pool, nullptr);

    vkDestroyFence(_device, _frames[i]._render_fence, nullptr);
    vkDestroySemaphore(_device, _frames[i]._render_semaphore, nullptr);
    vkDestroySemaphore(_device, _frames[i]._swapchain_semaphore, nullptr);

    _frames[i]._deletion_queue.flush();
  }
}

void MB_Cmd::init_commands() {
  VkCommandPoolCreateInfo command_pool_info{};
  command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.pNext = nullptr;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_info.queueFamilyIndex = _graphics_queue_family;

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    if (vkCreateCommandPool(
      _device, 
      &command_pool_info, 
      nullptr,
      &_frames[i]._command_pool
    ) != VK_SUCCESS) {
      throw std::runtime_error("failed to create command pool!");
    }

    VkCommandBufferAllocateInfo cmd_alloc_info{};
    cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc_info.pNext = nullptr;
    cmd_alloc_info.commandPool = _frames[i]._command_pool;
    cmd_alloc_info.commandBufferCount = 1;
    cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(_device, &cmd_alloc_info, &_frames[i]._main_command_buffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate buffer!");
    }

  }

  init_sync_structures();
}

void MB_Cmd::wait_for_render() {
  vkWaitForFences(_device, 1, &get_current_frame()._render_fence, true, 100000000);
  get_current_frame()._deletion_queue.flush();
  vkResetFences(_device, 1, &get_current_frame()._render_fence);
}

void MB_Cmd::begin_recording(VkCommandBufferUsageFlags flags) {
  current_cmd = get_current_frame()._main_command_buffer;
  vkResetCommandBuffer(current_cmd, 0);

  VkCommandBufferBeginInfo cmd_info{};
  cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_info.pNext = nullptr;
  cmd_info.pInheritanceInfo = nullptr;
  cmd_info.flags = flags;

  vkBeginCommandBuffer(current_cmd, &cmd_info);
}


void MB_Cmd::begin_renderpass(const VkRenderPassBeginInfo* render_info, VkSubpassContents contents) {
  vkCmdBeginRenderPass(current_cmd, render_info, contents);
}

void MB_Cmd::end_recording() {
  vkEndCommandBuffer(current_cmd);
}

void MB_Cmd::end_renderpass() {
  vkCmdEndRenderPass(current_cmd);
}

void MB_Cmd::submit_graphics(VkPipelineStageFlags2 wait_mask, VkPipelineStageFlags2 signal_mask) {
  VkSemaphoreSubmitInfo wait_info{};
	wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	wait_info.pNext = nullptr;
	wait_info.semaphore = get_current_frame()._swapchain_semaphore;
	wait_info.stageMask = wait_mask;
	wait_info.deviceIndex = 0;
	wait_info.value = 1;

  VkSemaphoreSubmitInfo signal_info{};
	signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	signal_info.pNext = nullptr;
	signal_info.semaphore = get_current_frame()._render_semaphore;
	signal_info.stageMask = signal_mask;
	signal_info.deviceIndex = 0;
	signal_info.value = 1;

  VkCommandBufferSubmitInfo cmd_info{};
  cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	cmd_info.pNext = nullptr;
	cmd_info.commandBuffer = current_cmd;
	cmd_info.deviceMask = 0;

  VkSubmitInfo2 submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  submit_info.pNext = nullptr;
  submit_info.waitSemaphoreInfoCount = &wait_info == nullptr ? 0 : 1;
  submit_info.pWaitSemaphoreInfos = &wait_info;
  submit_info.signalSemaphoreInfoCount = &signal_info == nullptr ? 0 : 1;
  submit_info.pSignalSemaphoreInfos = &signal_info;
  submit_info.commandBufferInfoCount = 1;
  submit_info.pCommandBufferInfos = &cmd_info;

  queue_submit(_device, _graphics_queue, 1, &submit_info, get_current_frame()._render_fence);
}

void MB_Cmd::present_graphics(VkSwapchainKHR _swapchain, uint32_t* swapchain_image_index) {
  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext = nullptr;
	present_info.pSwapchains = &_swapchain;
	present_info.swapchainCount = 1;

	present_info.pWaitSemaphores = &get_current_frame()._render_semaphore;
	present_info.waitSemaphoreCount = 1;

	present_info.pImageIndices = swapchain_image_index;

  if (vkQueuePresentKHR(_graphics_queue, &present_info) != VK_SUCCESS) {
    throw std::runtime_error("failed to present image!");
  }

  _frame_number++;
}

void MB_Cmd::copy_image_to_image(VkImage src, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size) {
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
    current_cmd, 
    src, 
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
    destination, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    1, 
    &blit_region, 
    VK_FILTER_LINEAR
  );
}

void MB_Cmd::draw_background(MB_Swapchain* mb_swapchain, VkExtent2D _window_extent, uint32_t image_index) {
  VkClearValue clear_value;
  float flash = static_cast<float>(abs(sin(_frame_number / 120.f)));
  clear_value.color = { { 0.0f, 0.0f, flash, 1.0f } };

  VkRenderPassBeginInfo renderpass_info{};
  renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderpass_info.pNext = nullptr;

  renderpass_info.renderPass = mb_swapchain->get_renderpass();
	renderpass_info.renderArea.offset.x = 0;
	renderpass_info.renderArea.offset.y = 0;
	renderpass_info.renderArea.extent = _window_extent;
	renderpass_info.framebuffer = mb_swapchain->get_framebuffers()[image_index];

  renderpass_info.clearValueCount = 1;
  renderpass_info.pClearValues = &clear_value;

  begin_renderpass(&renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
  end_renderpass();
}

void MB_Cmd::init_sync_structures() {
  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.pNext = nullptr;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphore_info.pNext = nullptr;
  semaphore_info.flags = 0;

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    vkCreateFence(_device, &fence_info, nullptr, &_frames[i]._render_fence);

    vkCreateSemaphore(_device, &semaphore_info, nullptr, &_frames[i]._swapchain_semaphore);
    vkCreateSemaphore(_device, &semaphore_info, nullptr, &_frames[i]._render_semaphore);
  }
}

} // namespace GRAPHICS
