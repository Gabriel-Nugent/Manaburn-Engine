#pragma once

#include "../vulkan_util/vk_types.h"

#include "device.h"
#include "swapchain.h"
#include "pipeline.h"
#include "cmd.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

//--- LOAD IN VULKAN EXTENSION FUNCTIONS ---//

// load in the function to create the debug messenger
static VkResult CreateDebugUtilsMessengerEXT(
  VkInstance instance, 
  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
  const VkAllocationCallbacks* pAllocator, 
  VkDebugUtilsMessengerEXT* pDebugMessenger
) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

// load in the function to destroy the debug messenger
static void DestroyDebugUtilsMessengerEXT(
  VkInstance instance, 
  VkDebugUtilsMessengerEXT debugMessenger, 
  const VkAllocationCallbacks* pAllocator
) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}


//--- VALIDATION LAYER INFO ---//

const std::vector<const char*> VALIDATION_LAYERS = {
  "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> instance_extensions = {
  "VK_KHR_get_physical_device_properties2"
};

#ifdef NDEBUG
  const bool ENABLE_VALIDATION_LAYERS = false;
#else 
  const bool ENABLE_VALIDATION_LAYERS = true;
#endif

class vk_interface {
  public:
    VkInstance   _instance;
    Device*      _device;
    Swapchain*   _swapchain;
    Cmd*         _cmd;
    VmaAllocator _allocator;
    
    vk_interface(SDL_Window* window) : _window(window) {};
    ~vk_interface();

    vk_interface (const vk_interface&) = delete;
    vk_interface& operator= (const vk_interface&) = delete;

    void init(VkExtent2D _window_extent) {
      init_instance();

      if (ENABLE_VALIDATION_LAYERS) {
        setup_debug_messenger();
      }

      create_surface();

      _device = new Device(_instance, _surface);

      init_allocator();

      _swapchain = new Swapchain(_instance, _device, _surface, _window, _allocator);
      _swapchain->init(_window_extent);

      _cmd = new Cmd(_device);
      _cmd->init_commands();

      _initialized = true;
    }

    uint32_t get_next_image();

    void draw_background(
      VkRenderPassBeginInfo* renderpass_info, 
      VkExtent2D _window_extent,
      uint32_t swapchain_image_index
    );

  private:
    bool _initialized = false;

    VkDebugUtilsMessengerEXT debug_messenger;

    SDL_Window*   _window;
    VkSurfaceKHR  _surface;

    VkClearValue clear_values[2];

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* pUserData
    );

    void setup_debug_messenger();

    void init_instance();
    void create_surface();
    void init_allocator();
};