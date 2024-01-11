#include "Pipeline.h"

namespace GRAPHICS
{

Pipeline::~Pipeline() {
  if (vert_shader != VK_NULL_HANDLE) {
    vkDestroyShaderModule(_device, frag_shader, nullptr);
    vkDestroyShaderModule(_device, vert_shader, nullptr);
    frag_shader = VK_NULL_HANDLE;
    vert_shader = VK_NULL_HANDLE;
  }
}

void Pipeline::clear() {
  // zero out all structs
  _input_assembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
  _vertex_input_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	_rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	_color_blend_attachment = {};
	_multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	_pipeline_layout = {};
	_shader_stages.clear();

  if (vert_shader != VK_NULL_HANDLE) {
    vkDestroyShaderModule(_device, frag_shader, nullptr);
    vkDestroyShaderModule(_device, vert_shader, nullptr);
    frag_shader = VK_NULL_HANDLE;
    vert_shader = VK_NULL_HANDLE;
  }
}

VkPipeline Pipeline::build_pipeline(VkRenderPass pass) {
  // viewport state will be created from the stored viewport and scissor
  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.pNext = nullptr;

  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  // dummy color blending
  VkPipelineColorBlendStateCreateInfo color_blending{};
  color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.pNext = nullptr;

  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY;
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &_color_blend_attachment;

  // build pipeline with pre built info structs
  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
  };
  //connect the renderInfo to the pNext extension mechanism
  pipeline_info.pNext = nullptr;

  pipeline_info.stageCount = (uint32_t)_shader_stages.size();
  pipeline_info.pStages = _shader_stages.data();
  pipeline_info.pVertexInputState = &_vertex_input_info;
  pipeline_info.pInputAssemblyState = &_input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &_rasterizer;
  pipeline_info.pMultisampleState = &_multisampling;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.layout = _pipeline_layout;
  pipeline_info.renderPass = pass;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

  // setup our dynamic states
  VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
  };
  dynamic_info.pDynamicStates = &state[0];
  dynamic_info.dynamicStateCount = 2;

  pipeline_info.pDynamicState = &dynamic_info;

  VkPipeline new_pipeline;
  if (vkCreateGraphicsPipelines(
    _device, 
    VK_NULL_HANDLE, 
    1, 
    &pipeline_info, 
    nullptr,
    &new_pipeline)
  != VK_SUCCESS) {
    fmt::println("failed to create pipeline");
  }
  else {
    vkDestroyShaderModule(_device, frag_shader, nullptr);
    vkDestroyShaderModule(_device, vert_shader, nullptr);
    frag_shader = VK_NULL_HANDLE;
    vert_shader = VK_NULL_HANDLE;
    return new_pipeline;
  }

  return VK_NULL_HANDLE;
}

void Pipeline::set_shaders(std::string vert_filepath, std::string frag_filepath) {
  _shader_stages.clear();
  
  // load in shader stage code
  auto vert_shader_code = read_file(vert_filepath);
  auto frag_shader_code = read_file(frag_filepath);
  vert_shader = create_shader_module(vert_shader_code);
  frag_shader = create_shader_module(frag_shader_code);

  VkPipelineShaderStageCreateInfo vert_shader_info{};
  vert_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_info.module = vert_shader;
  vert_shader_info.pName = "main";
  VkPipelineShaderStageCreateInfo frag_shader_info{};
  frag_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_info.module = frag_shader;
  frag_shader_info.pName = "main";

  _shader_stages.push_back(vert_shader_info);
  _shader_stages.push_back(frag_shader_info);
}

void Pipeline::set_vertex_input_info() {
  _vertex_input_info.vertexAttributeDescriptionCount = 0;
  _vertex_input_info.vertexBindingDescriptionCount = 0;
}

void Pipeline::set_input_topology(VkPrimitiveTopology topology) {
  _input_assembly.topology = topology;
  _input_assembly.primitiveRestartEnable = VK_FALSE;
}

void Pipeline::set_polygon_mode(VkPolygonMode mode) {
  _rasterizer.polygonMode = mode;
  _rasterizer.lineWidth = 1.f;
}

void Pipeline::set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face) {
  _rasterizer.cullMode = cull_mode;
  _rasterizer.frontFace = front_face;
}

void Pipeline::set_multisampling_none() {
  _multisampling.sampleShadingEnable = VK_FALSE;
  _multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  _multisampling.minSampleShading = 1.0f;
  _multisampling.pSampleMask = nullptr;
  _multisampling.alphaToCoverageEnable = VK_FALSE;
  _multisampling.alphaToOneEnable = VK_FALSE;
}

void Pipeline::set_pipeline_layout(VkPipelineLayout layout) {
  _pipeline_layout = layout;
}

void Pipeline::disable_blending() {
  _color_blend_attachment.colorWriteMask = 
    VK_COLOR_COMPONENT_R_BIT |
    VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT |
    VK_COLOR_COMPONENT_A_BIT;
  _color_blend_attachment.blendEnable = VK_FALSE;
}

std::vector<char> Pipeline::read_file(const std::string& filepath) {
  // read the file starting at the end in binary mode
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file: [" + filepath + "]!");
  }

  size_t file_size = (size_t) file.tellg();
  std::vector<char> buffer(file_size);

  // read the entire file into the buffer
  file.seekg(0);
  file.read(buffer.data(), file_size);

  file.close();

  return buffer;
}

VkShaderModule Pipeline::create_shader_module(const std::vector<char>& code) {
  VkShaderModuleCreateInfo shader_module_info{};
  shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shader_module_info.codeSize = code.size();
  shader_module_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shader_module;
  if (vkCreateShaderModule(
    _device,
    &shader_module_info, 
    nullptr, 
    &shader_module
    ) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
  }

  return shader_module;
}


} // namespace GRAPHICS
