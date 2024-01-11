#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct vkutil {

static glm::mat4 calc_render_matrix(const glm::vec3 camera_position,const int _frame_number);

};
