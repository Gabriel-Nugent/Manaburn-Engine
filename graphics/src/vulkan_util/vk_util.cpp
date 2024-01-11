#include "vk_util.h"

glm::mat4 vkutil::calc_render_matrix(const glm::vec3 camera_position,const int _frame_number) {
  glm::mat4 view = glm::translate(glm::mat4(1.f), camera_position);

  glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
  projection[1][1] *= -1;

  glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(_frame_number * 0.4f), glm::vec3(0, 1, 0));

  glm::mat4 mesh_matrix = projection * view * model;

  return mesh_matrix;
}