#include "StaticMesh.h"

#include <glad/glad.h>

namespace OM3D {

BoundingCriteria::BoundingCriteria(glm::vec3 aa, glm::vec3 bb, glm::vec3 center, float rad) : _aa(aa), _bb(bb), _center(center), _radius(rad) {}
BoundingCriteria::BoundingCriteria(const MeshData& data) {
    glm::vec3 center;
	for(auto& v : data.vertices) center += v.position / (float) data.vertices.size();
    _center = center;
	_radius = 0;
	for(auto& v : data.vertices) _radius = glm::max(_radius, glm::length(v.position - center));
    _aa = center, _bb = center;
    for(auto& v : data.vertices){
        _aa.x = std::min(_aa.x, v.position.x);
        _aa.y = std::min(_aa.y, v.position.y);
        _aa.z = std::min(_aa.z, v.position.z);
        _bb.x = std::max(_bb.x, v.position.x);
        _bb.y = std::max(_bb.y, v.position.y);
        _bb.z = std::max(_bb.z, v.position.z);
    }
}

const glm::vec3& BoundingCriteria::aa() const { return _aa; }
const glm::vec3& BoundingCriteria::bb() const { return _bb; }
const glm::vec3& BoundingCriteria::center() const { return _center; }
const float& BoundingCriteria::radius() const { return _radius; }

bool BoundingCriteria::test(glm::vec3 boxpos, glm::vec3 cp, const Frustum& frustum) const {
	auto pos = (boxpos+_center)-cp;
	if(glm::length(pos) <= _radius) return true;
	return glm::dot(pos, frustum._near_normal) >= -_radius && (
		glm::dot(pos, frustum._top_normal) >= -_radius &&
		glm::dot(pos, frustum._bottom_normal) >= -_radius &&
		glm::dot(pos, frustum._left_normal) >= -_radius &&
		glm::dot(pos, frustum._right_normal) >= -_radius &&
		true
	);
}

BoundingCriteria BoundingCriteria::transform_aabb(const glm::mat4& mat) const {
    auto mv3 = [&](glm::vec3 v){ return glm::vec3(mat * glm::vec4(v, 1.0)); };
    auto center = mv3(_center);
    auto aa = center;
    auto bb = center;
    std::array<glm::vec3, 8> verts{
        mv3(glm::vec3(_aa.x, _aa.y, _aa.z)),
        mv3(glm::vec3(_aa.x, _aa.y, _bb.z)),
        mv3(glm::vec3(_aa.x, _bb.y, _aa.z)),
        mv3(glm::vec3(_aa.x, _bb.y, _bb.z)),
        mv3(glm::vec3(_bb.x, _aa.y, _aa.z)),
        mv3(glm::vec3(_bb.x, _aa.y, _bb.z)),
        mv3(glm::vec3(_bb.x, _bb.y, _aa.z)),
        mv3(glm::vec3(_bb.x, _bb.y, _bb.z)),
    };
    float radius = 0;
    for(auto v : verts){
        aa.x = std::min(aa.x, v.x);
        aa.y = std::min(aa.y, v.y);
        aa.z = std::min(aa.z, v.z);
        bb.x = std::max(bb.x, v.x);
        bb.y = std::max(bb.y, v.y);
        bb.z = std::max(bb.z, v.z);
        radius = glm::max(radius, glm::length(v - center));
    }
    return BoundingCriteria(aa, bb, center, radius);
}

StaticMesh::StaticMesh(const MeshData& data) : _data(data), _bb(data) {}

const MeshData& StaticMesh::data() const { return _data; }
const BoundingCriteria& StaticMesh::bb() const { return _bb; }

bool StaticMesh::test(glm::vec3 pos, glm::vec3 camera, const Frustum& frustum) const {
	return _bb.test(pos, camera, frustum);
}

std::shared_ptr<StaticMeshUploaded> StaticMesh::upload() const {
    return std::make_shared<StaticMeshUploaded>(*this);
}

StaticMeshUploaded::StaticMeshUploaded(const StaticMesh& data) :
    _vertex_buffer(data._data.vertices),
    _index_buffer(data._data.indices),
    _bb(data._bb) {}

bool StaticMeshUploaded::test(glm::vec3 pos, glm::vec3 camera, const Frustum& frustum) const {
	return _bb.test(pos, camera, frustum);
}


std::size_t StaticMeshUploaded::DEBUG_VERTS_DRAWN = 0;
std::size_t StaticMeshUploaded::DEBUG_TRIS_DRAWN = 0;

void StaticMeshUploaded::draw() const {
    _vertex_buffer.bind(BufferUsage::Attribute);
    _index_buffer.bind(BufferUsage::Index);

    // Vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
    // Vertex normal
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(4 * sizeof(float)));
    // Vertex uv
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(8 * sizeof(float)));
    // Tangent / bitangent sign
    glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(12 * sizeof(float)));
    // Vertex color
    glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(16 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glDrawElements(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr);

    DEBUG_VERTS_DRAWN += _vertex_buffer.element_count();
    DEBUG_TRIS_DRAWN += _index_buffer.element_count()/3;
}

}
