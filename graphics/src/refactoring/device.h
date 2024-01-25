#pragma once

#include "vk_types.h"

#include <vulkan/vulkan.h>

const std::vector<const char*> device_extensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  "VK_KHR_synchronization2",
};

class Device
{
  public:
    Device(VkInstance instance, VkSurfaceKHR surface);
    ~Device();

    VkPhysicalDevice _physical = VK_NULL_HANDLE;
    VkDevice _logical = VK_NULL_HANDLE;

    std::optional<uint32_t> _graphics_index;
    VkQueue                 _graphics_queue;
    std::optional<uint32_t> _present_index;
    VkQueue                 _present_queue;
  private:
    VkInstance _instance; 
    VkSurfaceKHR _surface;

    void pick_physical_device();
    bool is_device_suitable(VkPhysicalDevice gpu);
    void find_queue_indices();
    void create_logical_device();
};
