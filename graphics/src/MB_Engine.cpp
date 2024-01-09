#include "MB_Engine.h"

#define VMA_IMPLEMENTATION
#include "../external_src/vk_mem_alloc.h"


namespace GRAPHICS
{
  
MB_Engine::~MB_Engine() {
  // cleanup has not been called manually and thus must be performed
  if (_initialized) {
    cleanup();
  }
}
  
void MB_Engine::init() {
  init_vulkan();

  _initialized = true;
}

/**
 * @brief main rendering loop for the MB_Engine, runs until
 *        window is closed for an error is encountered.
 */
void MB_Engine::run() {
SDL_Event event;
bool should_quit = false;

// main rendering loop
while (!should_quit) {
  // handle events
  while (SDL_PollEvent(&event) != 0) {
    // quit on alt-f4 or exit
    if (event.type == SDL_QUIT) {
      should_quit = true;
    }
    else if (event.type == SDL_WINDOWEVENT) {
      switch (event.window.type) {
        // rendering should halt while the window is minimized
        case SDL_WINDOWEVENT_MINIMIZED:
          stop_rendering = true;
          break;
        // rendering will resume when the window is maximized
        case SDL_WINDOWEVENT_MAXIMIZED:
          stop_rendering = false;
          break;

        default:
          break;
      }
    }
  }
  // slow loop iteration to save resources  
  if (stop_rendering) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    continue;
  }

  draw();
}

}

/**
 * @brief Frees all allocated memory for the MB_Engine
 *        and destroys all vulkan handles
 */
void MB_Engine::cleanup() {
  // 
  if (_initialized) {

    if (ENABLE_VALIDATION_LAYERS) {
      DestroyDebugUtilsMessengerEXT(_instance, debug_messenger, nullptr);
    }
    
    vkDeviceWaitIdle(device->get_device());
    _main_deletion_queue.flush();

    delete image;
    vmaDestroyAllocator(_allocator);

    delete cmd;
    delete swapchain;
    delete device;

    SDL_DestroyWindow(_window);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyInstance(_instance, nullptr);
  // Cleanup has been successfully performed
  _initialized = false;
  }
}

/**
 * @brief Initializes all of the MB_Engine members needed
 *        to render to the window
 */
void MB_Engine::init_vulkan() {
  create_window();
  create_instance();

  if (ENABLE_VALIDATION_LAYERS) {
    setup_debug_messenger();
  }

  create_surface();
  init_device();
  init_memory_allocator();
  create_swapchain();
  init_commands();
  init_pipelines();
}

/**
 * @brief Initializes a window using SDL
 */
void MB_Engine::create_window() {
  // we require a window from SDL
  SDL_Init(SDL_INIT_VIDEO);

  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

  _window = SDL_CreateWindow(
    "Manaburn MB_Engine",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    _window_extent.width,
    _window_extent.height,
    window_flags
  );
}

/**
 * @brief Initializes the Vulkan Instance needed to interact with the API
 */
void MB_Engine::create_instance() {
  // Vulkan version 1.2 must be supported
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = NULL;
  app_info.pApplicationName = "Manaburn";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
  app_info.engineVersion = VK_MAKE_VERSION(1, 2, 0);
  app_info.apiVersion = VK_API_VERSION_1_2;

  // query for extensions required to create SDL surface
  uint32_t extension_count = 0;
  SDL_Vulkan_GetInstanceExtensions(_window, &extension_count, nullptr);
  std::vector<const char*> extension_names(extension_count);
  SDL_Vulkan_GetInstanceExtensions(_window, &extension_count, extension_names.data());

  // add the debug messenger extension to the extension list
  if (ENABLE_VALIDATION_LAYERS) {
    extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  // add all instance extensions
  for (auto extension : instance_extensions) {
    extension_names.push_back(extension);
  }

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = static_cast<uint32_t>(extension_names.size());
  create_info.ppEnabledExtensionNames = extension_names.data();
  if (ENABLE_VALIDATION_LAYERS) {
    create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
    create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
  }
  else {
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = nullptr;
  }

  if (vkCreateInstance(&create_info, nullptr, &_instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }
}

/**
 * @brief Sets up the debug messenger that interprets the validation layers
 *        and outputs them to the console
 */
void MB_Engine::setup_debug_messenger() {
  VkDebugUtilsMessengerCreateInfoEXT debug_info{};
  debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debug_info.messageSeverity = 
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debug_info.messageType = 
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debug_info.pfnUserCallback = debugCallback;
  debug_info.pUserData = nullptr; // Optional

  if (CreateDebugUtilsMessengerEXT(_instance, &debug_info, nullptr, &debug_messenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

/**
 * @brief The window surface is necessary for Vulkan -> SDL interaction
 */
void MB_Engine::create_surface() {
  if (SDL_Vulkan_CreateSurface(_window, _instance, &_surface) != SDL_TRUE) {
    throw std::runtime_error("Failed to create surface!");
  }
}

/**
 * @brief Initializes the physical device, logical device, and submission queues
 */
void MB_Engine::init_device() {
  device = new Device(_instance, _surface);
  device->init();
  _device = device->get_device();
}

/**
 * @brief Initializes AMD's Vulkan Memory Allocator
 */
void MB_Engine::init_memory_allocator() {
  VmaAllocatorCreateInfo allocator_info{};

  allocator_info.physicalDevice = device->get_gpu();
  allocator_info.device = device->get_device();
  allocator_info.instance = _instance;
  allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  vmaCreateAllocator(&allocator_info, &_allocator);
}

/**
 * @brief Initializes the swaphchain and its resources:
 *          Images, Image Views, Renderpass, Framebuffers
 */
void MB_Engine::create_swapchain() {
  swapchain = new Swapchain(_instance, device, _surface, _window);
  swapchain->create_default();
  swapchain->init_default_renderpass();
  swapchain->init_framebuffers();

  image = new Image(_allocator, device->get_device(), _window_extent);
}

/**
 * @brief Initializes the command buffers and the synchronization structures
 * 
 */
void MB_Engine::init_commands() {
  cmd = new Cmd(device);
  cmd->init_commands();
}

void MB_Engine::init_pipelines() {
  init_triangle_pipeline();
}

void MB_Engine::init_triangle_pipeline() {
  Pipeline pipeline(device->get_device());
}

/**
 * @brief Images are retrieved from the swapchain, drawn on,
 *        and then presented to the window surface
 */
void MB_Engine::draw() {
  // wait for the previous frame to finish rendering
  cmd->wait_for_render();

  // grab the next image from the swaphchain
  uint32_t swapchain_image_index;
  vkAcquireNextImageKHR(
    _device, 
    swapchain->get_swapchain(),
    1000000000,
    cmd->get_current_frame()._swapchain_semaphore,
    nullptr, 
    &swapchain_image_index
  );
  
  cmd->begin_recording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  cmd->draw_background(swapchain, _window_extent, swapchain_image_index);

  cmd->end_recording();

  // submit the image to the graphics queue
  cmd->submit_graphics(
    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
    VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
  );

  // present the graphics image to the window
  cmd->present_graphics(swapchain->get_swapchain(), &swapchain_image_index);
  _frame_number++;

}

} // namespace MB
