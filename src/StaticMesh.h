#ifndef STATICMESH_H
#define STATICMESH_H

#include <graphics.h>
#include <TypedBuffer.h>
#include <Vertex.h>

#include <vector>
#include <Camera.h>

#include <memory>

namespace OM3D {

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
};

class BoundingCriteria {
	public:
		BoundingCriteria() = default;
		BoundingCriteria(glm::vec3 aa, glm::vec3 bb, glm::vec3 center, float);
		BoundingCriteria(const MeshData&);

        const glm::vec3& aa() const;
        const glm::vec3& bb() const;
        const glm::vec3& center() const;
        const float& radius() const;

		bool test(glm::vec3 pos, glm::vec3 camera, const Frustum& frustum) const;

        BoundingCriteria transform_aabb(const glm::mat4& mat) const;
    friend class GPULODGen;
	private:
		glm::vec3 _aa, _bb;
		glm::vec3 _center;
		float _radius;
};

class StaticMesh;
class StaticMeshUploaded;

class StaticMesh {
    private:
        MeshData _data;
        BoundingCriteria _bb;
    public:
        StaticMesh() = default;
        StaticMesh(const MeshData& data);

        const MeshData& data() const;
        const BoundingCriteria& bb() const;

        bool test(glm::vec3 pos, glm::vec3 camera, const Frustum& frustum) const;

        std::shared_ptr<StaticMeshUploaded> upload() const;

    friend class StaticMeshUploaded;
};

class StaticMeshUploaded : NonCopyable {
    public:
        StaticMeshUploaded() = default;
        StaticMeshUploaded(StaticMeshUploaded&&) = default;
        StaticMeshUploaded& operator=(StaticMeshUploaded&&) = default;

        StaticMeshUploaded(const StaticMesh& meshs);

		bool test(glm::vec3 pos, glm::vec3 camera, const Frustum& frustum) const;
        void draw() const;

        static std::size_t DEBUG_VERTS_DRAWN;
        static std::size_t DEBUG_TRIS_DRAWN;

    friend class StaticMesh;
    private:
        TypedBuffer<Vertex> _vertex_buffer;
        TypedBuffer<u32> _index_buffer;
		BoundingCriteria _bb;
};

}

#endif // STATICMESH_H
