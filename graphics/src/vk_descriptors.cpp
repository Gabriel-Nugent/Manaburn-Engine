#include "vk_descriptors.h"

namespace vklayout
{

void Layout::triangle_layout(VkDevice _device, VkPipelineLayout* layout) {
  VkPipelineLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  info.pNext = nullptr;
  // empty defaults
  info.flags = 0;
  info.setLayoutCount = 0;
  info.pSetLayouts = nullptr;
  info.pushConstantRangeCount = 0;
  info.pPushConstantRanges = nullptr;

  VK_CHECK(vkCreatePipelineLayout(_device, &info, nullptr, layout));
}

} // namespace vklayout
