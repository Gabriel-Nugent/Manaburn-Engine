#pragma once

#include "vk_types.h"
#include "device.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

struct Swapchain_details {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

class Swapchain {
  public:
    Swapchain(
      VkInstance  instance, 
      Device* device,
      VkSurfaceKHR surface,
      SDL_Window* window,
      VmaAllocator allocator
    );
    ~Swapchain();

    void init(VkExtent2D _window_extent) {
      create_default();
      init_depth_image(_window_extent);
      init_default_renderpass();
      init_framebuffers();
    }

    // Swapchain handles
    VkSwapchainKHR            _swapchain;
    VkSwapchainCreateInfoKHR  old_create_info{};
    Swapchain_details         details;
    VkFormat                  swapchain_image_format;
    Image*                    _depth_image;
    uint32_t                  image_count;
    VkExtent2D                swapchain_extent;
    std::vector<VkImage>      swapchain_images;
    std::vector<VkImageView>  swapchain_image_views;

    // Secondary handles
    VkRenderPass               _renderpass;
	  std::vector<VkFramebuffer> _framebuffers;

    VkRenderPassBeginInfo renderpass_begin_info(VkExtent2D _window_extent, uint32_t swapchain_image_index);

  private:
    VkInstance    _instance;
    Device*       _device;
    VkSurfaceKHR  _surface;
    SDL_Window*   _window;
    VmaAllocator  _allocator;

    VkSwapchainKHR _old_swapchain = VK_NULL_HANDLE;

    void create_default();
    void init_default_renderpass();
    void init_framebuffers();
    void init_depth_image(VkExtent2D _window_extent);

    void query_swapchain_details();
    VkSurfaceFormatKHR choose_surface_format();
    VkPresentModeKHR choose_present_mode();
    VkExtent2D choose_extent();
    void create_image_views();
};