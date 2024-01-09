#pragma once

#include <vector>

#include "MB_Device.h"

static VkResult queue_submit(VkDevice _device, VkQueue queue, uint32_t submitCount, 
const VkSubmitInfo2* pSubmits, VkFence fence) {
  auto func = (PFN_vkQueueSubmit2KHR) vkGetDeviceProcAddr(_device, "vkQueueSubmit2KHR");
  if (func != nullptr) {
    return func(queue, submitCount, pSubmits, fence);
  }
  else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

namespace GRAPHICS
{

/**
 * @brief queue for holding memory objects that need to 
 *        be released when they are no longer needed
 */
struct DeletionQueue {
  std::vector<void*> queue;

  void add_pointer(void* ptr) {
    queue.push_back(ptr);
  }

  void flush() {
    for (auto ptr : queue) {
      delete ptr;
    }
  }
};

struct Frame_Data {
  VkCommandPool _command_pool;
  VkCommandBuffer _main_command_buffer;
  VkSemaphore _swapchain_semaphore, _render_semaphore;
  VkFence _render_fence;
  DeletionQueue _deletion_queue;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class MB_Cmd
{
public:
  MB_Cmd(MB_Device* device);
  ~MB_Cmd();

  Frame_Data& get_current_frame() { 
    return _frames[_frame_number % FRAME_OVERLAP];
  }

  void init_commands();
  void wait_for_render();
  void begin_recording(VkCommandBufferUsageFlags flags);
  void begin_renderpass(const VkRenderPassBeginInfo* render_info, VkSubpassContents contents);
  void end_recording();
  void end_renderpass();

  void submit_graphics(VkPipelineStageFlags2 wait_mask, VkPipelineStageFlags2 signal_mask);
  void present_graphics(VkSwapchainKHR _swapchain, uint32_t* swapchain_image_index);
private:
  VkDevice _device;
  VkQueue _graphics_queue;
  uint32_t _graphics_queue_family;

  int _frame_number{0};
  Frame_Data _frames[FRAME_OVERLAP];

  VkRenderPass _renderpass;
  std::vector<VkFramebuffer> _framebuffers;

  VkCommandBuffer current_cmd;

  void init_sync_structures();
};

} // namespace GRAPHICS


