#ifndef SHADERSTRUCTS_H
#define SHADERSTRUCTS_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/matrix.hpp>

namespace OM3D {
namespace shader {

using namespace glm;

#define _GLSL_ALIGN_ alignas(16)
#define _GLSL_CPP_
#include <../shaders/structs.glsl>

}
}

#endif // SHADERSTRUCTS_H
