#pragma once

#include "vk_types.h"

#include "device.h"

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
    Device* _device;
    swapchain* _swapchain;

    vk_interface(SDL_Window* window) : _window(window) {};
    ~vk_interface();

    vk_interface (const vk_interface&) = delete;
    vk_interface& operator= (const vk_interface&) = delete;

    void init() {
      init_instance();
      if (ENABLE_VALIDATION_LAYERS) {
        setup_debug_messenger();
      }
      _device = new Device(_instance, _surface);
      init_allocator();
    }

  private:
    VkDebugUtilsMessengerEXT debug_messenger;
    SDL_Window* _window;
    VkInstance _instance;
    VkSurfaceKHR _surface;
    VmaAllocator _allocator;

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