#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "vk_types.h"

namespace vklayout
{

class Layout {
public:
  static void triangle_layout(VkDevice _device, VkPipelineLayout* layout);
  static void mesh_layout(VkDevice _device, VkPipelineLayout* layout);

private:
  struct MeshPushConstants {
    glm::vec4 data;
    glm::mat4 render_matrix;
  };
};

} // namespace vklayout
