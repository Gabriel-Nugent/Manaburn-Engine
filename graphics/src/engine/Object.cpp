#include "Object.h"

VertexInputDescription Vertex::get_vertex_description() {
  VertexInputDescription description;

  //we will have just 1 vertex buffer binding, with a per-vertex rate
	VkVertexInputBindingDescription main_binding = {};
	main_binding.binding = 0;
	main_binding.stride = sizeof(Vertex);
	main_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(main_binding);

	//Position will be stored at Location 0
	VkVertexInputAttributeDescription position_attribute = {};
	position_attribute.binding = 0;
	position_attribute.location = 0;
	position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_attribute.offset = offsetof(Vertex, position);

	//Normal will be stored at Location 1
	VkVertexInputAttributeDescription normal_attribute = {};
	normal_attribute.binding = 0;
	normal_attribute.location = 1;
	normal_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normal_attribute.offset = offsetof(Vertex, normal);

	//Color will be stored at Location 2
	VkVertexInputAttributeDescription color_attribute = {};
	color_attribute.binding = 0;
	color_attribute.location = 2;
	color_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	color_attribute.offset = offsetof(Vertex, color);

	description.attributes.push_back(position_attribute);
	description.attributes.push_back(normal_attribute);
	description.attributes.push_back(color_attribute);
	return description;
}

void Material::create_material(VkPipeline pipeline, VkPipelineLayout layout) {
  _pipeline = pipeline;
  _pipelineLayout = layout;
}

Object::Object(const char* filename, VmaAllocator allocator) : _allocator(allocator) {
  bool result = load_obj(filename);
}

Object::Object(Mesh cpy_mesh, VmaAllocator allocator) : _allocator(allocator), mesh(cpy_mesh){}

Object::~Object() {
  if (is_uploaded) {
    vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
  }
}

bool Object::load_obj(const char* filename) {

  tinyobj::attrib_t attrib; // vertex arrays for file
  std::vector<tinyobj::shape_t> shapes; // contains info for each object in file
  std::vector<tinyobj::material_t> materials; // materials for each shape

  std::string warn;
  std::string err;

  tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, nullptr);

  // handle errors and warnings
  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }
  if (!err.empty()) {
    std::cerr << err << std::endl;
    return false;
  }

  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      // hardcoded loading of triangles
      int fv = 3;

      for (size_t v = 0; v < fv; v++) {
        // get access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        // get vertex position
        tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
        // get vertex normal
        //vertex normal
        tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

        // copy to vertex struct
        Vertex new_vert;
        new_vert.position.x = vx;
				new_vert.position.y = vy;
				new_vert.position.z = vz;

				new_vert.normal.x = nx;
				new_vert.normal.y = ny;
        new_vert.normal.z = nz;

        //temporarily setting vertex color as the vertex normal
        new_vert.color = new_vert.normal;

        mesh._vertices.push_back(new_vert);
      }
      index_offset += fv;
    }
  }

  return true;
}

void Object::upload_mesh() {
  VkBufferCreateInfo buffer_info {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

  buffer_info.size = mesh._vertices.size() * sizeof(Vertex);

  buffer_info.usage =  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;;

  VmaAllocationCreateInfo vma_alloc_info {};
  vma_alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  VK_CHECK(vmaCreateBuffer(_allocator, &buffer_info, &vma_alloc_info,
    &mesh._vertexBuffer._buffer,
    &mesh._vertexBuffer._allocation,
    nullptr
  ));

  void* data;
  vmaMapMemory(_allocator, mesh._vertexBuffer._allocation, &data);

  memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));

  vmaUnmapMemory(_allocator, mesh._vertexBuffer._allocation);

  is_uploaded = true;
}