#include "MB_GUI.h"

namespace GRAPHICS
{

void MB_GUI::init() {
  // create command pool
  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.pNext = nullptr;
  pool_info.queueFamilyIndex = _graphics_queue_index;
  pool_info.flags =  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  vkCreateCommandPool(_device, &pool_info, nullptr, &_command_pool);

  // create command buffer
  VkCommandBufferAllocateInfo cmd_alloc_info{};
  cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd_alloc_info.pNext = nullptr;
  cmd_alloc_info.commandPool = _command_pool;
  cmd_alloc_info.commandBufferCount = 1;
  cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  vkAllocateCommandBuffers(_device, &cmd_alloc_info, &_command_buffer);

  // initialize fence
  VkFenceCreateInfo fence_create_info{};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_create_info.pNext = nullptr;
  fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  vkCreateFence(_device, &fence_create_info, nullptr, &_fence);
}


void MB_GUI::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function) {
  vkResetFences(_device, 1, &_fence);
  vkResetCommandBuffer(_command_buffer, 0);

  VkCommandBuffer cmd = _command_buffer;

  VkCommandBufferBeginInfo cmd_begin_info{};
  cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_begin_info.pNext = nullptr;
  cmd_begin_info.pInheritanceInfo = nullptr;
  cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmd, &cmd_begin_info);

  function(cmd);

  vkEndCommandBuffer(cmd);
  
}


} // namespace GRAPHICS
