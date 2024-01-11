#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "vk_types.h"

namespace vklayout
{

class Layout {
public:
  static void triangle_layout(VkDevice _device, VkPipelineLayout* layout);
};

} // namespace vklayout
