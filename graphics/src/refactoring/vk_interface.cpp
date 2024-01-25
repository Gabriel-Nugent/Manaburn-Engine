#include "vk_interface.h"

/**
 * @brief Initializes the Vulkan Instance needed to interact with the API
 */
void vk_interface::init_instance() {
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

  VK_CHECK(vkCreateInstance(&create_info, nullptr, &_instance));
}

void vk_interface::create_surface() {
  if (SDL_Vulkan_CreateSurface(_window, _instance, &_surface) != SDL_TRUE) {
    throw std::runtime_error("Failed to create surface!");
  }
}

/**
 * @brief Sets up the debug messenger that interprets the validation layers
 *        and outputs them to the console
 */
void vk_interface::setup_debug_messenger() {
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

  VK_CHECK(CreateDebugUtilsMessengerEXT(_instance, &debug_info, nullptr, &debug_messenger));
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_interface::debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* pUserData
) {
    
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