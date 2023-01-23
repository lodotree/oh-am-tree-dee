#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

namespace OM3D {

SceneObject::SceneObject(){}
SceneObject::SceneObject(StaticMesh mesh, std::shared_ptr<Material> material) : _mesh(mesh), _material(material) {}

const StaticMesh& SceneObject::mesh() const { return _mesh; }
std::shared_ptr<Material> SceneObject::material() const { return _material; }


void SceneObject::set_transform(const glm::mat4& tr) {
    _transform = tr;
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}

glm::vec3 SceneObject::get_transform_center() const {
    return glm::vec3(_transform[3]);
}

bool SceneObject::test(glm::vec3 camera, const Frustum& frustum) const {
    return _mesh.test(get_transform_center(), camera, frustum);
}

SceneObject SceneObject::genlod(const GPULODGen& lod, float cellsize) const {
    auto so = SceneObject(lod.gen_lod(_mesh, cellsize), _material);
    so.set_transform(_transform);
    return so;
}

SceneObject SceneObject::mk_aabb(const BoundingCriteria& box, glm::vec3 color){
    const auto aa = box.aa();
    const auto bb = box.bb();
    return SceneObject(StaticMesh(MeshData{
        .vertices = {
            Vertex{ .position=glm::vec3(aa.x, aa.y, aa.z), .color=color },
            Vertex{ .position=glm::vec3(bb.x, aa.y, aa.z), .color=color },
            Vertex{ .position=glm::vec3(bb.x, bb.y, aa.z), .color=color },
            Vertex{ .position=glm::vec3(aa.x, bb.y, aa.z), .color=color },
            Vertex{ .position=glm::vec3(aa.x, aa.y, bb.z), .color=color },
            Vertex{ .position=glm::vec3(bb.x, aa.y, bb.z), .color=color },
            Vertex{ .position=glm::vec3(bb.x, bb.y, bb.z), .color=color },
            Vertex{ .position=glm::vec3(aa.x, bb.y, bb.z), .color=color },
        },
        .indices = {
            0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            5, 4, 6, 6, 4, 7,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1
        },
    }), Material::empty_material());
}

SceneObjectUploaded SceneObject::upload() const {
    return SceneObjectUploaded(*this);
}

SceneObjectUploaded::SceneObjectUploaded(const SceneObject& o) :
    _mesh(std::move(o._mesh.upload())),
    _material(std::move(o._material)),
    _transform(o._transform) {}

bool SceneObjectUploaded::test(glm::vec3 camera, const Frustum& frustum) const {
	return _mesh->test(glm::vec3(_transform[3]), camera, frustum);
}

void SceneObjectUploaded::render(bool shade) const {
    if(!_material || !_mesh) {
        return;
    }

    _material->set_uniform(HASH("model"), transform());
    _material->bind(shade);
    _mesh->draw();
}

void SceneObjectUploaded::set_transform(const glm::mat4& tr) {
    _transform = tr;
}

const glm::mat4& SceneObjectUploaded::transform() const {
    return _transform;
}

}
