#include "gui.h"

GUI::~GUI() {
  vkDestroyDescriptorPool(vk->_device->_logical, imgui_pool, nullptr);
  ImGui_ImplVulkan_Shutdown();
}

void GUI::init_imgui() {
  VkDescriptorPoolSize pool_sizes[] = { 
    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

  VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

  VK_CHECK(vkCreateDescriptorPool(vk->_device->_logical, &pool_info, nullptr, &imgui_pool));

  //--- INITIALIZE IMGUI ---//
  ImGui::CreateContext();
  //ImGui::SetCurrentContext(ctx);
  ImGui_ImplSDL2_InitForVulkan(_window); // imgui for SDL2

  // imgui for vulkan
  ImGui_ImplVulkan_InitInfo init_info {};
  init_info.Instance = vk->_instance;
	init_info.PhysicalDevice = vk->_device->_physical;
	init_info.Device = vk->_device->_logical;
	init_info.Queue = vk->_device->_graphics_queue;
	init_info.DescriptorPool = imgui_pool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.UseDynamicRendering = false;
	init_info.ColorAttachmentFormat = vk->_swapchain->swapchain_image_format;

  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

  ImGui_ImplVulkan_Init(&init_info, vk->_swapchain->_renderpass);

  vk->_cmd->immediate_submit([&](VkCommandBuffer cmd) { ImGui_ImplVulkan_CreateFontsTexture(cmd); });

  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void GUI::process_event(SDL_Event *event) {
  ImGui_ImplSDL2_ProcessEvent(event);
}

void GUI::begin_drawing() {
  // imgui new frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL2_NewFrame(_window);
  ImGui::NewFrame();

  // some imgui UI to test
  ImGui::ShowDemoWindow();

  // make imgui calculate internal draw structures
  ImGui::Render();
}

void GUI::draw_imgui() {
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vk->_cmd->current_cmd);
}