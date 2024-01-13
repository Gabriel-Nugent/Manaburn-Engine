#pragma once 

#include "../vulkan_util/vk_types.h"

#include <vector>
#include <iostream>
#include <unordered_map>

#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>
#include <tiny_obj_loader.h>

namespace GRAPHICS {

struct VertexInputDescription {
  std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 color;

  static VertexInputDescription get_vertex_description();
};

struct Mesh {
  std::vector<Vertex> _vertices;

  AllocatedBuffer _vertexBuffer;

  bool load_from_obj(const char* filename);
};

class Object {
public:
  Mesh mesh;

  Object(const char* filename, VmaAllocator allocator);
  Object(Mesh cpy_mesh, VmaAllocator allocator);
  ~Object();

  void upload_mesh();
private:
  bool is_uploaded = false;

  VmaAllocator _allocator;
};

} // namespace GRAPHICS