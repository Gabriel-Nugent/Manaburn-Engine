#pragma once

#include <vulkan/vulkan.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl2.h"
#include "../imgui/imgui_impl_vulkan.h"

#include <functional>

#include "../vulkan_util/vk_types.h"
#include "../vulkan/vk_interface.h"

class GUI
{
public:
  GUI(SDL_Window* window, vk_interface* _vk) : _window(window), vk(_vk) {};
  ~GUI();

  void init_imgui();
  void process_event(SDL_Event *event);
  void begin_drawing();
  void draw_imgui();

private:
  vk_interface* vk;
  SDL_Window* _window;

  VkDescriptorPool imgui_pool;
};