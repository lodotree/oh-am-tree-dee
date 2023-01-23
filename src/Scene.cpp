#include "Scene.h"

#include <TypedBuffer.h>

#include <shader_structs.h>
#include <unordered_map>

namespace OM3D {

Scene::Scene() {
}

void Scene::add_object(SceneObject obj) {
    _objects.emplace_back(std::move(obj));
}

void Scene::add_object(PointLight obj) {
    _point_lights.emplace_back(std::move(obj));
}

void Scene::render(const Camera& camera) const {
	auto frustum = camera.build_frustum();

    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = camera.view_proj_matrix();
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
    light_buffer.bind(BufferUsage::Storage, 1);

    // Render every object
    std::unordered_map<std::shared_ptr<Material>, std::vector<const SceneObject*>> objects;
    for(const SceneObject& obj : _objects) if(obj.test(camera.position(), frustum)) {
        if(obj._material->is_instanced()) objects[obj._material].push_back(&obj);
        else obj.render();
    }
    for(const auto& mos : objects){
        mos.first->bind();
        const auto instances = mos.second.size();
        std::vector<glm::mat4> mats(instances);
        for(std::size_t i = 0; i < instances; i++) mats[i] = mos.second[i]->transform();
        TypedBuffer<glm::mat4> modeltrmat_buffer(mats);
        modeltrmat_buffer.bind(BufferUsage::Storage, 10);
        mos.second[0]->_mesh->draw(instances);
    }
}

}
