#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
  Camera(){ pos = {0.f, 0.f, -2.f}; }
  Camera(glm::vec3 starting_position){ pos = starting_position; }
  ~Camera(){};

  // the position of the camera in space
  glm::vec3 pos;
};

