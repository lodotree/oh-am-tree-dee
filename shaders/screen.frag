#version 450
//! Fragment shader for g-buffer population

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_albedo;
layout(binding = 1) uniform sampler2D in_normal;
layout(binding = 2) uniform sampler2D in_depth;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 10) buffer PointLights {
    PointLight point_lights[];
};

vec3 unproject(vec2 uv, float depth, mat4 inv_viewproj) {
    const vec3 ndc = vec3(uv * 2.0 - vec2(1.0), depth);
    const vec4 p = inv_viewproj * vec4(ndc, 1.0);
    return p.xyz / p.w;
}

void main(){
    float depth = texelFetch(in_depth, ivec2(gl_FragCoord.x, gl_FragCoord.y), 0).x;
	vec3 albedo = texelFetch(in_albedo, ivec2(gl_FragCoord.x, gl_FragCoord.y), 0).xyz;
	vec3 normal = ((texelFetch(in_normal, ivec2(gl_FragCoord.x, gl_FragCoord.y), 0) - 0.5) * 2.).xyz;
    if(depth < 1E-8) out_color = vec4(albedo, 1.0); //Nothing is here, yeet clear color
    else {
        vec3 wpos = unproject(in_uv, depth, frame.camera.inv_view_proj);
        
        vec3 acc = frame.sun_color * max(0.0, dot(frame.sun_dir, normal));
        for(uint i = 0; i != frame.point_light_count; ++i) {
            PointLight light = point_lights[i];
            const vec3 to_light = (light.position - wpos);
            const float dist = length(to_light);
            const vec3 light_vec = to_light / dist;

            const float NoL = dot(light_vec, normal);
            const float att = attenuation(dist, light.radius);
            if(NoL <= 0.0 || att <= 0.0f) {
                continue;
            }

            acc += light.color * (NoL * att);
        }

        out_color = vec4(albedo, 1.0);
        // out_color = vec4(albedo*acc, 1.0);
        // out_color = vec4(frame.sun_color, 1.0);
        // out_color = vec4(depth*1000, depth*1000, depth*1000, 1.0);
        // out_color = vec4(((-1. + wpos)/2.).x, depth*1000, depth*1000, 1.0);
    }
}
