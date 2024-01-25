#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "../vulkan_util/vk_types.h"
#include "../vulkan_util/vk_util.h"
#include "../vulkan_util/vk_descriptors.h"

#include "../vulkan/pipeline.h"
#include "../vulkan/vk_interface.h"
#include "gui.h"
#include "object.h"
#include "camera.h"

struct Obj_Queue {
  std::unordered_map<std::string, Object*> map;

  void flush() {
    for (auto obj : map) {
      delete obj.second;
    }
  }
};

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
  DeletionQueue _main_deletion_queue;

  // Graphics Pipelines handles
  Pipeline_Queue pipeline_queue;

  // Engine objects
  Obj_Queue mb_objs;
  std::unordered_map<std::string, Material> materials;
  std::vector<Object*> _renderables;

  // Wrapper handles
  vk_interface* vk;
  Pipeline* pipeline;
  GUI* gui;

  // camera and movement states
  Camera* camera;
  float x_velocity;
  float y_velocity;

  void create_window();

  void init_pipelines();
  void init_mesh_pipeline();

  void init_gui();

  void load_meshes();

  void init_camera();

  void init_scene();

  void draw();

};
