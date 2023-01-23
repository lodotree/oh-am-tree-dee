#ifndef STATICMESH_H
#define STATICMESH_H

#include <graphics.h>
#include <TypedBuffer.h>
#include <Vertex.h>

#include <vector>
#include <Camera.h>

namespace OM3D {

class BoundingCriteria {
	public:
		BoundingCriteria() = default;
		BoundingCriteria(glm::vec3, float);
		bool test(glm::vec3 pos, glm::vec3 camera, const Frustum& frustum) const;
	private:
		glm::vec3 _center;
		float _radius;
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
};

class StaticMesh : NonCopyable {

    public:
        StaticMesh() = default;
        StaticMesh(StaticMesh&&) = default;
        StaticMesh& operator=(StaticMesh&&) = default;

        StaticMesh(const MeshData& data);

		bool test(glm::vec3 pos, glm::vec3 camera, const Frustum& frustum) const;
        void draw() const;

    private:
        TypedBuffer<Vertex> _vertex_buffer;
        TypedBuffer<u32> _index_buffer;
		BoundingCriteria _bb;
};

}

#endif // STATICMESH_H
