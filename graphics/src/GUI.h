#pragma once

#include <vulkan/vulkan.h>

#include <functional>

namespace GRAPHICS
{

class GUI
{
public:
  GUI(VkDevice device);
  ~GUI(){};

  void init();
  void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
private:
  VkDevice _device;
  VkQueue _graphics_queue;
  uint32_t _graphics_queue_index;

  VkFence _fence;
  VkCommandBuffer _command_buffer;
  VkCommandPool _command_pool;
};

} // namespace: GRAPHICS
