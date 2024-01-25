#pragma once

#include <vulkan/vulkan.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl2.h"
#include "../imgui/imgui_impl_vulkan.h"

#include <functional>

#include "../vulkan_util/vk_types.h"
#include "../vulkan/Cmd.h"
#include "../vulkan/Device.h"

namespace GRAPHICS
{

class GUI
{
public:
  GUI(Device* dev, Swapchain* swap, SDL_Window* window, Cmd* cm);
  ~GUI();

  void init_imgui(VkInstance _instance);
  void process_event(SDL_Event *event);
  void begin_drawing();
  void draw_imgui();

private:
  Device* device;
  Swapchain* swapchain;
  Cmd* cmd;
  SDL_Window* _window;

  VkDescriptorPool imgui_pool;
};

} // namespace: GRAPHICS
