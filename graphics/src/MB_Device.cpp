#include "MB_Device.h"

namespace GRAPHICS {

MB_Device::MB_Device(VkInstance instance, VkSurfaceKHR surface)
: _instance(instance), _surface(surface) {}

MB_Device::~MB_Device() {
  vkDestroyDevice(_device, nullptr);
}

void MB_Device::init() {
  pick_physical_device();
  find_queue_indices();
  create_logical_device();
}

void MB_Device::pick_physical_device() {
  // query for physical devices
  uint32_t physical_device_count = 0;
  vkEnumeratePhysicalDevices(_instance, &physical_device_count, nullptr);
  std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
  vkEnumeratePhysicalDevices(_instance, &physical_device_count, physical_devices.data());

  // enumerate over gpus to pick the correct one
  for (auto gpu : physical_devices) {
    if(is_device_suitable(gpu)) {
      _chosen_GPU = gpu;
      break;
    }
  }

  if (_chosen_GPU == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find suitable gpu!");
  }
}

bool MB_Device::is_device_suitable(VkPhysicalDevice gpu) {
  // check for the extensions available on the device
  uint32_t extension_count = 0;
  vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count, nullptr);
  std::vector<VkExtensionProperties> available_extensions(extension_count);
  vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count, available_extensions.data());

  int extensions_unmatched = static_cast<int>(device_extensions.size());
  
  for (auto available_extension : available_extensions) {
    for (auto extension_name : device_extensions ) {
      if (strcmp(extension_name, available_extension.extensionName) == 0) {
        extensions_unmatched--;
      }
    }
  }

  if (extensions_unmatched == 0) {
    return true;
  }

  return false;
}

void MB_Device::find_queue_indices() {
  // query for physical device properties
  uint32_t property_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(_chosen_GPU, &property_count, nullptr);
  std::vector<VkQueueFamilyProperties> properties(property_count);
  vkGetPhysicalDeviceQueueFamilyProperties(_chosen_GPU, &property_count, properties.data());

  for (uint32_t i = 0; i < property_count; i++) {
    if (!_graphics_queue_index.has_value() 
    && (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      // grab first queue family that supports graphics
      _graphics_queue_index = i;
    }
    // query for present support
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(_chosen_GPU, i, _surface, &present_support);
    if (!_present_queue_index.has_value() && present_support) {
      _present_queue_index = i;
    }
    // break early if both indices have been filled
    if (_present_queue_index.has_value() && _graphics_queue_index.has_value()) {
      break;
    }
  }
}


void MB_Device::create_logical_device() {
  // queue create infos

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set<uint32_t> unique_queue_families = {
    _graphics_queue_index.value(), _present_queue_index.value()
  };

  float queue_priority = 1.0f;
  for (uint32_t queue_family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_info{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueFamilyIndex = queue_family;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_info);
  }

  VkPhysicalDeviceFeatures device_features{};

  // enable synchronization 2 features for the device
  VkPhysicalDeviceSynchronization2FeaturesKHR sync_features{};
  sync_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
  sync_features.pNext = nullptr;
  sync_features.synchronization2 = VK_TRUE;

  VkDeviceCreateInfo device_info{};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = &sync_features;
  device_info.pQueueCreateInfos = queue_create_infos.data();
  device_info.queueCreateInfoCount = 
    static_cast<uint32_t>(queue_create_infos.size());
  device_info.pEnabledFeatures = &device_features;
  device_info.enabledExtensionCount 
    = static_cast<uint32_t>(device_extensions.size());
  device_info.ppEnabledExtensionNames = device_extensions.data();

  if (vkCreateDevice(_chosen_GPU, &device_info, nullptr, &_device) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(_device, _graphics_queue_index.value(), 0, &_graphics_queue);
  vkGetDeviceQueue(_device, _present_queue_index.value(), 0, &_present_queue);
}

} // namespace GRAPHICS