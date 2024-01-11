#pragma once

#include <vector>

#include "vk_types.h"
#include "Device.h"
#include "Swapchain.h"

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

class Cmd
{
public:
  Cmd(Device* device);
  ~Cmd();

  Frame_Data& get_current_frame() { 
    return _frames[_frame_number % FRAME_OVERLAP];
  }

  void init_commands();
  void wait_for_render();
  void begin_recording(VkCommandBufferUsageFlags flags);
  void begin_renderpass(VkRenderPassBeginInfo* begin_info, VkSubpassContents contents);
  void bind_pipeline(VkPipeline pipeline, VkPipelineBindPoint bind_point);
  void set_window(const VkExtent2D _window_extent);
  void draw_geometry(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
  void end_recording();
  void end_renderpass();

  void submit_graphics(VkPipelineStageFlags2 wait_mask, VkPipelineStageFlags2 signal_mask);
  void present_graphics(VkSwapchainKHR _swapchain, uint32_t* swapchain_image_index);

  void transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout);
  void copy_image_to_image(VkImage src, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size);
  void draw_background(Swapchain* swapchain, VkExtent2D _window_extent, uint32_t image_index);
private:
  VkDevice _device;
  VkQueue _graphics_queue;
  uint32_t _graphics_queue_family;

  int _frame_number{0};
  Frame_Data _frames[FRAME_OVERLAP];

  VkRenderPass _renderpass;
  VkRenderPassBeginInfo current_renderpass_info;
  std::vector<VkFramebuffer> _framebuffers;

  VkCommandBuffer current_cmd;

  void init_sync_structures();
};

} // namespace GRAPHICS


