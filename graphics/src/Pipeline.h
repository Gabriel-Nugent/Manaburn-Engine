#pragma once

#include <vector>
#include <string>
#include <fstream>

#include <vulkan/vulkan.h>
#include <fmt/core.h>

#include "Device.h"

namespace GRAPHICS
{

struct Pipeline_Queue {
  std::vector<VkPipeline> pipelines;
  std::vector<VkPipelineLayout> pipeline_layouts;

  void flush(VkDevice _device) {
    for (VkPipeline pipeline : pipelines) {
      vkDestroyPipeline(_device, pipeline, nullptr);
    }
    for (VkPipelineLayout layout : pipeline_layouts) {
      vkDestroyPipelineLayout(_device, layout, nullptr);
    }
  }
};

class Pipeline
{
public:
  std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;
  VkPipelineVertexInputStateCreateInfo _vertex_input_info;
  VkPipelineInputAssemblyStateCreateInfo _input_assembly;
  VkPipelineRasterizationStateCreateInfo _rasterizer;
  VkPipelineColorBlendAttachmentState _color_blend_attachment;
  VkPipelineMultisampleStateCreateInfo _multisampling;
  VkPipelineLayout _pipeline_layout;

  Pipeline(VkDevice device){ 
    clear();
    _device = device;
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
  void disable_blending();

private:
  VkDevice _device;
  VkShaderModule vert_shader = VK_NULL_HANDLE;
  VkShaderModule frag_shader = VK_NULL_HANDLE;

  static std::vector<char> read_file(const std::string& filepath);
  VkShaderModule create_shader_module(const std::vector<char>& code);
  
};

} // namespace GRAPHICS
