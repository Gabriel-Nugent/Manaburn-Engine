#pragma once

#include <vector>
#include <string>
#include <fstream>

#include <vulkan/vulkan.h>
#include <fmt/core.h>

#include "MB_Device.h"

namespace GRAPHICS
{

class MB_Pipeline
{
public:
  std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;

  VkPipelineInputAssemblyStateCreateInfo _input_assembly;
  VkPipelineRasterizationStateCreateInfo _rasterizer;
  VkPipelineColorBlendAttachmentState _color_blend_attachment;
  VkPipelineMultisampleStateCreateInfo _multisampling;
  VkPipelineLayout _pipeline_layout;
  VkPipelineDepthStencilStateCreateInfo _depth_stencil;
  VkPipelineRenderingCreateInfo _render_info;
  VkFormat _color_attachmentformat;

  MB_Pipeline(VkDevice device){ 
    clear();
    _device = device;
  }

  ~MB_Pipeline();

  void clear();

  VkPipeline build_pipeline();
  static void create_pipeline_layout(VkPipelineLayout* layout);

  void set_shaders(std::string vert_filepath, std::string frag_filepath);
  void set_input_topology(VkPrimitiveTopology topology);
  void set_polygon_mode(VkPolygonMode mode);
  void set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face);
  void set_multisampling_none();
  void disable_blending();
  void set_color_attachment_format(VkFormat format);
  void set_depth_format(VkFormat format);
  void disable_depthtest();

private:
  VkDevice _device;
  VkShaderModule vert_shader = VK_NULL_HANDLE;
  VkShaderModule frag_shader = VK_NULL_HANDLE;

  static std::vector<char> read_file(const std::string& filepath);
  VkShaderModule create_shader_module(const std::vector<char>& code);
  
};

} // namespace GRAPHICS
