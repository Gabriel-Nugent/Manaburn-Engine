#pragma once

#include "../vulkan_util/vk_types.h"

#include "device.h"
#include "swapchain.h"
#include "../engine/object.h"


// Load in queue submit vulkan function
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

struct MeshPushConstants {
  glm::vec4 data;
  glm::mat4 render_matrix;
};

/**
 * @brief queue for holding memory objects that need to 
 *        be released when they are no longer needed
 */
struct DeletionQueue {
  std::vector<Mesh*> meshes;

  void flush_meshes(VmaAllocator _allocator) {
    for (auto mesh : meshes) {
      vmaDestroyBuffer(_allocator, mesh->_vertexBuffer._buffer, mesh->_vertexBuffer._allocation);
    }
  }
};

struct Frame_Data {
  VkCommandPool   _command_pool;
  VkCommandBuffer _main_command_buffer;
  VkSemaphore     _swapchain_semaphore, _render_semaphore;
  VkFence         _render_fence;
  DeletionQueue   _deletion_queue;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class Cmd
{
public:
  Cmd(Device* _device);
  ~Cmd();

  VkCommandBuffer current_cmd;

  Frame_Data& get_current_frame() { 
    return _frames[_frame_number % FRAME_OVERLAP];
  }

  // handles for immediate submit commands
  VkFence         _imm_fence;
  VkCommandBuffer _imm_command_buffer;
  VkCommandPool   _imm_command_pool;

  void init_commands();
  void wait_for_render();
  void begin_recording(VkCommandBufferUsageFlags flags);
  void begin_renderpass(VkRenderPassBeginInfo* begin_info, VkSubpassContents contents);
  void bind_pipeline(VkPipeline pipeline, VkPipelineBindPoint bind_point);
  void set_window(const VkExtent2D _window_extent);

  void set_push_constants(VkPipelineLayout layout, VkShaderStageFlags flags, uint32_t offset, uint32_t size, const void* push_values);
  void draw_objects(glm::vec3 cam_pos, Object** first, size_t count);
  void draw_geometry(Mesh* mesh, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);

  void end_recording();
  void end_renderpass();

  void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
  void submit_graphics(VkPipelineStageFlags2 wait_mask, VkPipelineStageFlags2 signal_mask);
  void present_graphics(VkSwapchainKHR _swapchain, uint32_t* swapchain_image_index);

  void transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout);
  void copy_image_to_image(VkImage src, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size);
  void draw_background(Swapchain* swapchain, VkExtent2D _window_extent, uint32_t image_index);
private:
  VkDevice  _logical;
  VkQueue   _graphics_queue;
  uint32_t  _graphics_queue_family;

  int         _frame_number{0};
  Frame_Data  _frames[FRAME_OVERLAP];

  VkRenderPass               _renderpass;
  VkRenderPassBeginInfo      current_renderpass_info;
  std::vector<VkFramebuffer> _framebuffers;

  void init_sync_structures();
};


