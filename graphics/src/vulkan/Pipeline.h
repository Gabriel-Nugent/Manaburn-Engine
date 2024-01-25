#pragma once

#include "../vulkan_util/vk_types.h"

#include "Device.h"

struct Pipeline_Queue {
  std::unordered_map<std::string, VkPipeline> pipelines;
  std::unordered_map<std::string, VkPipelineLayout> pipeline_layouts;

  void flush(VkDevice _logical) {
    for (auto pipeline : pipelines) {
      vkDestroyPipeline(_logical, pipeline.second, nullptr);
    }
    for (auto layout : pipeline_layouts) {
      vkDestroyPipelineLayout(_logical, layout.second, nullptr);
    }
  }
};

class Pipeline
{
public:
  std::vector<VkPipelineShaderStageCreateInfo>  _shader_stages;
  VkPipelineVertexInputStateCreateInfo          _vertex_input_info;
  VkPipelineInputAssemblyStateCreateInfo        _input_assembly;
  VkPipelineRasterizationStateCreateInfo        _rasterizer;
  VkPipelineColorBlendAttachmentState           _color_blend_attachment;
  VkPipelineMultisampleStateCreateInfo          _multisampling;
  VkPipelineLayout                              _pipeline_layout;
  VkPipelineDepthStencilStateCreateInfo         _depth_stencil;

  Pipeline(VkDevice device){ 
    clear();
    _logical = device;
  }

  ~Pipeline();

  void clear();

  VkPipeline build_pipeline(VkRenderPass pass);

  void set_shaders(std::string vert_filepath, std::string frag_filepath);
  void set_vertex_input_info();
  void set_input_topology(VkPrimitiveTopology topology);
  void set_polygon_mode(VkPolygonMode mode);
  void set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face);
  void set_multisampling_none();
  void set_pipeline_layout(VkPipelineLayout layout);
  void default_depth_stencil(bool depth_test, bool depth_write, VkCompareOp compareOp);
  void disable_blending();

private:
  VkDevice _logical;
  VkShaderModule vert_shader = VK_NULL_HANDLE;
  VkShaderModule frag_shader = VK_NULL_HANDLE;

  static std::vector<char> read_file(const std::string& filepath);
  VkShaderModule create_shader_module(const std::vector<char>& code);
  
};
