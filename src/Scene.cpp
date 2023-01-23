#include "Scene.h"

#include <TypedBuffer.h>

#include <shader_structs.h>
#include <algorithm>

namespace OM3D {

Scene::Scene(){}

void Scene::add_object(SceneObject obj) {
    _objects.emplace_back(std::move(obj));
}

void Scene::add_object(Chunk obj) {
    _chunks.emplace_back(std::move(obj));
}

void Scene::add_object(PointLight obj) {
    _point_lights.emplace_back(std::move(obj));
}

Scene Scene::genlod(const GPULODGen& lod, float cellsize) const {
    Scene s;
    s._sun_direction = _sun_direction;
    s._point_lights = _point_lights;
    std::transform(_objects.begin(), _objects.end(), std::back_inserter(s._objects), [&](const auto& obj){ return obj.genlod(lod, cellsize); });
    return s;
}

std::unique_ptr<SceneUploaded> Scene::upload() const {
    return std::make_unique<SceneUploaded>(*this);
}

SceneUploaded::SceneUploaded(const Scene& scene) : _point_lights(scene._point_lights), _sun_direction(scene._sun_direction) {
    _objects.reserve(scene._objects.size());
    for(std::size_t i = 0; i < scene._objects.size(); i++) _objects.push_back(scene._objects[i].upload());
    _chunks.reserve(scene._chunks.size());
    for(std::size_t i = 0; i < scene._chunks.size(); i++) _chunks.push_back(scene._chunks[i].upload());
}

void SceneUploaded::bind_light_buffer(const Camera& camera, u32 idx) const {
	auto frustum = camera.build_frustum();

    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = camera.view_proj_matrix();
        mapping[0].camera.inv_view_proj = glm::inverse(camera.view_proj_matrix());
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    buffer.bind(BufferUsage::Uniform, 0);

    // Fill and bind lights buffer
    TypedBuffer<shader::PointLight> light_buffer(nullptr, std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = light_buffer.map(AccessType::WriteOnly);
        for(size_t i = 0; i != _point_lights.size(); ++i) {
            const auto& light = _point_lights[i];
            mapping[i] = {
                light.position(),
                light.radius(),
                light.color(),
                0.0f
            };
        }
    }
    light_buffer.bind(BufferUsage::Storage, idx);
}

void SceneUploaded::render(const Camera& camera, bool shade, bool chunks_force_fullres) const {
	auto frustum = camera.build_frustum();

    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = camera.view_proj_matrix();
        mapping[0].camera.inv_view_proj = glm::inverse(camera.view_proj_matrix());
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    buffer.bind(BufferUsage::Uniform, 0);

    // Render chunks
    for(const auto& c : _chunks) if(c.test(camera.position(), frustum)) c.render(camera.view_proj_matrix(), camera.position(), frustum, shade, chunks_force_fullres);

    // Render every object
    for(const SceneObjectUploaded& obj : _objects) if(obj.test(camera.position(), frustum)) {
        obj.render(shade);
    }
}

}
