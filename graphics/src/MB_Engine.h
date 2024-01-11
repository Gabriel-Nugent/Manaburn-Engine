#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <fmt/core.h>

#include <vector>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>

#include "vk_types.h"
#include "vk_descriptors.h"
#include "Device.h"
#include "Swapchain.h"
#include "Image.h"
#include "Pipeline.h"
#include "Cmd.h"

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

//--- BEGIN CLASS DEFINITION ---//

namespace GRAPHICS
{

class MB_Engine
{
public:
  MB_Engine();
  ~MB_Engine();

  void init();
  void run();
  void cleanup();

private:
  // MB_Engine states and callbacks
  VkDebugUtilsMessengerEXT debug_messenger;
  bool _initialized { false };
  int _frame_number { 0 } ;
  bool stop_rendering { false };
  bool resize_requested{ false };
  int _selected_shader { 0 };

  // MB_Engine handles
  SDL_Window* _window;
  VkExtent2D _window_extent{ 1200 , 600 };
  VkInstance _instance;
  VkSurfaceKHR _surface;
  VkDevice _device;
  DeletionQueue _main_deletion_queue;
  VmaAllocator _allocator;

  // Graphics Pipelines handles
  Pipeline_Queue pipeline_queue;
  VkPipeline _triangle_pipeline;
  VkPipelineLayout _triangle_layout;
  VkPipeline _colored_pipeline;

  // Wrapper handles
  Device* device;
  Swapchain* swapchain;
  Image* image;
  Cmd* cmd;
  Pipeline* pipeline;

  void init_vulkan();
  void create_window();
  void create_instance();
  void setup_debug_messenger();
  void create_surface();
  void init_device();

  void init_memory_allocator();

  void create_swapchain();
  void init_commands();

  void init_pipelines();
  void init_triangle_pipeline();
  void init_colored_pipeline();

  void draw();

  // debug callback function
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    // only show messages of Warning or error level
    if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      return VK_FALSE;
    }
    // store message severity
    std::string severity;
    if (messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      severity = "WARNING";
    }
    else {
      severity = "ERROR";
    }
    // output message to error log
    fmt::print(stderr, "[{}]\n{}\n",severity, pCallbackData->pMessage);
    return VK_FALSE;
  }
};


} // namespace MB
