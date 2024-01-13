#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>

#include "../../external_src/vk_mem_alloc.h"
#include "Device.h"
#include "Image.h"

namespace GRAPHICS
{

struct Swapchain_details {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

class Swapchain {
public:
  Swapchain(
    VkInstance instance, 
    Device* device,
    VkSurfaceKHR surface,
    SDL_Window* window,
    VmaAllocator allocator
  );
  ~Swapchain();

  VkSwapchainKHR get_swapchain(){return _swapchain;}
  VkFormat get_format(){return swapchain_image_format;}
  std::vector<VkImage> get_images(){return swapchain_images;}
  VkRenderPass get_renderpass(){return _renderpass;}
  std::vector<VkFramebuffer> get_framebuffers(){return _framebuffers;}

  void create_default();
  void resize(VkExtent2D _window_extent);

  void init_default_renderpass();
  void init_framebuffers();
  void init_depth_image(VkExtent2D _window_extent);

  VkRenderPassBeginInfo renderpass_begin_info(VkExtent2D _window_extent, uint32_t swapchain_image_index);

  // image handles
  Image* _depth_image;
private:
  // Vulkan handles
  VkInstance _instance;
  VkPhysicalDevice _gpu;
  VkSurfaceKHR _surface;
  SDL_Window* _window;
  Device* device;
  VmaAllocator _allocator;

  // Swapchain handles
  VkSwapchainKHR _swapchain;
  VkSwapchainKHR _old_swapchain = VK_NULL_HANDLE;
  Swapchain_details details;
  VkSwapchainCreateInfoKHR old_create_info{};
  VkFormat swapchain_image_format;
  VkExtent2D swapchain_extent;
  std::vector<VkImage> swapchain_images;
  std::vector<VkImageView> swapchain_image_views;

  // Secondary handles
  VkRenderPass _renderpass;
	std::vector<VkFramebuffer> _framebuffers;

  void query_swapchain_details();
  VkSurfaceFormatKHR choose_surface_format();
  VkPresentModeKHR choose_present_mode();
  VkExtent2D choose_extent();
  void create_image_views();

};

} // namespace MB
