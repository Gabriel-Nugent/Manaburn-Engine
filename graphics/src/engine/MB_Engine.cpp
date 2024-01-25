#include "MB_Engine.h"

#include <glm/gtx/transform.hpp>

#define VMA_IMPLEMENTATION
#include "../../external_src/vk_mem_alloc.h"


namespace GRAPHICS
{

MB_Engine::MB_Engine() {

}

MB_Engine::~MB_Engine() {
  // cleanup has not been called manually and thus must be performed
  if (_initialized) {
    cleanup();
  }
}
  
void MB_Engine::init() {
  init_vulkan();
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

    if (ENABLE_VALIDATION_LAYERS) {
      DestroyDebugUtilsMessengerEXT(_instance, debug_messenger, nullptr);
    }
    
    vkDeviceWaitIdle(_device);
    mb_objs.flush();
    pipeline_queue.flush(_device);
    delete camera;
    delete gui;
    delete cmd;
    delete swapchain;
    vmaDestroyAllocator(_allocator);
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

  VK_CHECK(vkCreateInstance(&create_info, nullptr, &_instance));
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

  VK_CHECK(CreateDebugUtilsMessengerEXT(_instance, &debug_info, nullptr, &debug_messenger));
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
  VK_CHECK(vmaCreateAllocator(&allocator_info, &_allocator));
}

/**
 * @brief Initializes the swapchain and its resources:
 *          Images, Image Views, Renderpass, Framebuffers, Depth Images
 */
void MB_Engine::create_swapchain() {
  swapchain = new Swapchain(_instance, device, _surface, _window, _allocator);
  swapchain->create_default();
  swapchain->init_depth_image(_window_extent);
  swapchain->init_default_renderpass();
  swapchain->init_framebuffers();
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
  init_mesh_pipeline();
}

void MB_Engine::init_mesh_pipeline() {
  VkPipelineLayout layout;
  vklayout::Layout::mesh_layout(_device, &layout);

  Pipeline pipeline_builder(device->get_device());
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
  VkPipeline pipeline = pipeline_builder.build_pipeline(swapchain->get_renderpass());

  pipeline_queue.pipeline_layouts["Mesh Layout"] = layout;
  pipeline_queue.pipelines["Mesh Pipeline"] = pipeline;

  Material mat;
  mat.create_material(pipeline, layout);
  materials["mesh"] = mat;
}

void MB_Engine::init_gui() {
  gui = new GUI(device, swapchain, _window, cmd);
  gui->init_imgui(_instance);
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
  Object* triangle_obj = new Object(_triangle_mesh, _allocator);
  Object* monkey_obj = new Object("meshes/monkey_smooth.obj", _allocator);
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
  cmd->wait_for_render();

  // grab the next image from the swaphchain
  uint32_t swapchain_image_index;
  VK_CHECK(vkAcquireNextImageKHR(
    _device, 
    swapchain->get_swapchain(),
    1000000000,
    cmd->get_current_frame()._swapchain_semaphore,
    nullptr, 
    &swapchain_image_index
  ));
  
  cmd->begin_recording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  //cmd->draw_background(swapchain, _window_extent, swapchain_image_index);

  VkClearValue clear_value;
  clear_value.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
  VkClearValue depth_clear;
  depth_clear.depthStencil.depth = 1.f;

  VkRenderPassBeginInfo renderpass_info = swapchain->renderpass_begin_info(_window_extent, swapchain_image_index);
  renderpass_info.clearValueCount = 2;
  VkClearValue clear_values[] = { clear_value, depth_clear };
  renderpass_info.pClearValues = &clear_values[0];

  cmd->begin_renderpass(&renderpass_info, VK_SUBPASS_CONTENTS_INLINE);

  //--- RENDERING COMMANDS ---//
  cmd->set_window(_window_extent);
  cmd->draw_objects(camera->pos, _renderables.data(), _renderables.size());
  gui->draw_imgui();

  cmd->end_renderpass();
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
