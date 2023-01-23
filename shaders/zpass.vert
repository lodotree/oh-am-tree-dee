#version 450
//! Fragment shader for z-pass population

#include "utils.glsl"

layout(location = 0) in vec3 in_pos;

layout(binding = 0) uniform Data {
    FrameData frame;
};

uniform mat4 model;

void main() {
    const vec4 position = model * vec4(in_pos, 1.0);

    gl_Position = frame.camera.view_proj * position;
}
