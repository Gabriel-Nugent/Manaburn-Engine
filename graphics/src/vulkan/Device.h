#pragma once

#include <vulkan/vulkan.h>
  
#include <vector>
#include <stdexcept>
#include <optional>
#include <set>
#include <string.h>

const std::vector<const char*> device_extensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  "VK_KHR_synchronization2",
};

namespace GRAPHICS
{

class Device
{
public:
  Device(VkInstance instance, VkSurfaceKHR surface);
  ~Device();

  VkPhysicalDevice get_gpu(){return _chosen_GPU;}
  VkDevice get_device(){return _device;}
  VkQueue get_graphics_queue(){return _graphics_queue;}
  VkQueue get_present_queue(){return _present_queue;}
  uint32_t get_graphics_index(){return _graphics_queue_index.value();}
  uint32_t get_present_index(){return _present_queue_index.value();}

  void init();
private:
  VkInstance _instance;
  VkSurfaceKHR _surface;
  VkPhysicalDevice _chosen_GPU = VK_NULL_HANDLE;
  VkDevice _device;
  std::optional<uint32_t> _graphics_queue_index;
  VkQueue _graphics_queue;
  std::optional<uint32_t> _present_queue_index;
  VkQueue _present_queue;


  void pick_physical_device();
  bool is_device_suitable(VkPhysicalDevice gpu);
  void find_queue_indices();
  void create_logical_device();
};

} // namespace GRAPHICS
