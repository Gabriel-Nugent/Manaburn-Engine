#include "engine.h"

#include <glm/gtx/transform.hpp>

#define VMA_IMPLEMENTATION
#include "../../external_src/vk_mem_alloc.h"


MB_Engine::MB_Engine() {

}

MB_Engine::~MB_Engine() {
  // cleanup has not been called manually and thus must be performed
  if (_initialized) {
    cleanup();
  }
}
  
void MB_Engine::init() {
  create_window();
  vk = new vk_interface(_window);
  vk->init(_window_extent);
  init_pipelines();
  load_meshes();
  init_gui();
  init_camera();
  init_scene();
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
    // handles camera movement
    else if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
        case SDLK_a:
          camera->pos.x += 0.2f;
          break;

        case SDLK_d:
          camera->pos.x -= 0.2f;
          break;

        case SDLK_w:
          camera->pos.y -= 0.2f;
          break;

        case SDLK_s:
          camera->pos.y += 0.2f;
          break;

        default:
          break;
      }
    }
    else if (event.type == SDL_MOUSEWHEEL) {
      camera->pos.z += (event.wheel.preciseY * 0.5f);
    }

    gui->process_event(&event);
  }

  // slow loop iteration to save resources  
  if (stop_rendering) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    continue;
  }

  gui->begin_drawing();

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
    
    vkDeviceWaitIdle(vk->_device->_logical);
    mb_objs.flush();
    pipeline_queue.flush(vk->_device->_logical);
    delete camera;
    delete gui;
    
    SDL_DestroyWindow(_window);

    delete vk;
  // Cleanup has been successfully performed
  _initialized = false;
  }
}

/**
 * @brief Initializes a window using SDL
 */
void MB_Engine::create_window() {
  // we require a window from SDL
  SDL_Init(SDL_INIT_VIDEO);

  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  _window = SDL_CreateWindow(
    "Manaburn MB_Engine",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    _window_extent.width,
    _window_extent.height,
    window_flags
  );
}

void MB_Engine::init_pipelines() {
  init_mesh_pipeline();
}

void MB_Engine::init_mesh_pipeline() {
  VkPipelineLayout layout;
  vklayout::Layout::mesh_layout(vk->_device->_logical, &layout);

  Pipeline pipeline_builder(vk->_device->_logical);
  pipeline_builder.set_shaders("shaders/tri_mesh.vert.spv", "shaders/colored_triangle.frag.spv");

  VertexInputDescription vertex_description = Vertex::get_vertex_description();
  pipeline_builder._vertex_input_info.pVertexAttributeDescriptions = vertex_description.attributes.data();
  pipeline_builder._vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_description.attributes.size());
  pipeline_builder._vertex_input_info.pVertexBindingDescriptions = vertex_description.bindings.data();
  pipeline_builder._vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_description.bindings.size());

  pipeline_builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipeline_builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
  pipeline_builder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
  pipeline_builder.set_multisampling_none();
  pipeline_builder.set_pipeline_layout(layout);
  pipeline_builder.default_depth_stencil(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
  pipeline_builder.disable_blending();
  VkPipeline pipeline = pipeline_builder.build_pipeline(vk->_swapchain->_renderpass);

  pipeline_queue.pipeline_layouts["Mesh Layout"] = layout;
  pipeline_queue.pipelines["Mesh Pipeline"] = pipeline;

  Material mat;
  mat.create_material(pipeline, layout);
  materials["mesh"] = mat;
}

void MB_Engine::init_gui() {
  gui = new GUI(_window, vk);
  gui->init_imgui();
}

void MB_Engine::load_meshes() {
  // create triangle mesh for testing
  Mesh _triangle_mesh;
  _triangle_mesh._vertices.resize(3);
  _triangle_mesh._vertices.resize(3);

	//vertex positions
	_triangle_mesh._vertices[0].position = { 1.f, 1.f, 0.0f };
	_triangle_mesh._vertices[1].position = {-1.f, 1.f, 0.0f };
	_triangle_mesh._vertices[2].position = { 0.f,-1.f, 0.0f };

	//vertex colors, all green
  //we don't care about the vertex normals
	_triangle_mesh._vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
	_triangle_mesh._vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	_triangle_mesh._vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green

  // upload objects to the GPU
  Object* triangle_obj = new Object(_triangle_mesh, vk->_allocator);
  Object* monkey_obj = new Object("meshes/monkey_smooth.obj", vk->_allocator);
  triangle_obj->material = materials["mesh"];
  monkey_obj->material = materials["mesh"];
  mb_objs.map["Triangle"] = triangle_obj;
  mb_objs.map["Monkey"] = monkey_obj;
  triangle_obj->upload_mesh();
  monkey_obj->upload_mesh();
}

void MB_Engine::init_camera() {
  camera = new Camera();
}

void MB_Engine::init_scene() {
  Object* monkey = mb_objs.map["Monkey"];
  monkey->transform_mtx = glm::mat4{ 1.0f };
  _renderables.push_back(monkey);
}

/**
 * @brief Images are retrieved from the swapchain, drawn on,
 *        and then presented to the window surface
 */
void MB_Engine::draw() {
  // wait for the previous frame to finish rendering
  vk->_cmd->wait_for_render();

  // grab the next image from the swaphchain
  uint32_t swapchain_image_index = vk->get_next_image();
  
  vk->_cmd->begin_recording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  VkRenderPassBeginInfo renderpass_info {};
  vk->draw_background(&renderpass_info, _window_extent ,swapchain_image_index);

  vk->_cmd->begin_renderpass(&renderpass_info, VK_SUBPASS_CONTENTS_INLINE);

  //--- RENDERING COMMANDS ---//
  vk->_cmd->set_window(_window_extent);
  vk->_cmd->draw_objects(camera->pos, _renderables.data(), _renderables.size());
  gui->draw_imgui();

  vk->_cmd->end_renderpass();
  vk->_cmd->end_recording();

  // submit the image to the graphics queue
  vk->_cmd->submit_graphics(
    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
    VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
  );

  // present the graphics image to the window
  vk->_cmd->present_graphics(vk->_swapchain->_handle, &swapchain_image_index);
  _frame_number++;

}
