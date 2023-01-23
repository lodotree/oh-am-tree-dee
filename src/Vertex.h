#ifndef VERTEX_H
#define VERTEX_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace OM3D {

struct Vertex {
    alignas(sizeof(glm::vec4)) glm::vec3 position;
    alignas(sizeof(glm::vec4)) glm::vec3 normal;
    alignas(sizeof(glm::vec4)) glm::vec2 uv;
    alignas(sizeof(glm::vec4)) glm::vec4 tangent_bitangent_sign = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    alignas(sizeof(glm::vec4)) glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); // to avoid completly black meshes if no color is present
};

}

#endif // VERTEX_H
