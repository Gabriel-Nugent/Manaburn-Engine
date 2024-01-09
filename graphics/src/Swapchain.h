#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>

#include "Device.h"

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
    SDL_Window* window
  );
  ~Swapchain();

  VkSwapchainKHR get_swapchain(){return _swapchain;}
  VkFormat get_format(){return swapchain_image_format;}
  std::vector<VkImage> get_images(){return swapchain_images;}
  VkRenderPass get_renderpass(){return _renderpass;}
  std::vector<VkFramebuffer> get_framebuffers(){return _framebuffers;}

  void create_default();
  void create();
  void recreate();

  void init_default_renderpass();
  void init_framebuffers();
private:
  // Vulkan handles
  VkInstance _instance;
  VkPhysicalDevice _gpu;
  VkSurfaceKHR _surface;
  SDL_Window* _window;
  VkSwapchainKHR _swapchain;

  Device* device;
  Swapchain_details details;
  VkSwapchainCreateInfoKHR old_create_info{};
  VkFormat swapchain_image_format;
  VkExtent2D swapchain_extent;
  std::vector<VkImage> swapchain_images;
  std::vector<VkImageView> swapchain_image_views;

  VkRenderPass _renderpass;
	std::vector<VkFramebuffer> _framebuffers;

  void query_swapchain_details();
  VkSurfaceFormatKHR choose_surface_format();
  VkPresentModeKHR choose_present_mode();
  VkExtent2D choose_extent();
  void create_image_views();

};

} // namespace MB
