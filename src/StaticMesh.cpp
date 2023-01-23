#include "StaticMesh.h"

#include <glad/glad.h>

namespace OM3D {

BoundingCriteria::BoundingCriteria(glm::vec3 center, float rad) : _center(center), _radius(rad) {}

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

StaticMesh::StaticMesh(const MeshData& data) :
    _vertex_buffer(data.vertices),
    _index_buffer(data.indices) {
	glm::vec3 center;
	for(auto& v : data.vertices) center += v.position / (float) data.vertices.size();
	float radius = 0;
	for(auto& v : data.vertices) radius = glm::max(radius, glm::length(v.position - center));
	_bb = BoundingCriteria(center, radius);
}

bool StaticMesh::test(glm::vec3 pos, glm::vec3 camera, const Frustum& frustum) const {
	return _bb.test(pos, camera, frustum);
}

void StaticMesh::draw() const {
    _vertex_buffer.bind(BufferUsage::Attribute);
    _index_buffer.bind(BufferUsage::Index);

    // Vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
    // Vertex normal
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
    // Vertex uv
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
    // Tangent / bitangent sign
    glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(8 * sizeof(float)));
    // Vertex color
    glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(12 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glDrawElements(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr);
}

}
