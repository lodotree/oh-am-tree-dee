#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

namespace OM3D {

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material) :
    _mesh(std::move(mesh)),
    _material(std::move(material)) {
}

bool SceneObject::test(glm::vec3 camera, const Frustum& frustum) const {
	return _mesh->test(glm::vec3(_transform[3]), camera, frustum);
}

void SceneObject::render(bool shade) const {
    if(!_material || !_mesh) {
        return;
    }

   _material->set_uniform(HASH("model"), transform());
   _material->bind(shade);
    _mesh->draw();
}

void SceneObject::set_transform(const glm::mat4& tr) {
    _transform = tr;
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}

}
