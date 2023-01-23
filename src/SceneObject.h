#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <StaticMesh.h>
#include <Material.h>
#include <GPULODGen.h>

#include <memory>

#include <glm/matrix.hpp>

namespace OM3D {

class SceneObject;
class SceneObjectUploaded;

class SceneObject {
    private:
        glm::mat4 _transform = glm::mat4(1.0f);

        StaticMesh _mesh;
        std::shared_ptr<Material> _material;

    public:
        SceneObject();
        SceneObject(StaticMesh mesh, std::shared_ptr<Material> material = nullptr);

        const StaticMesh& mesh() const;
        std::shared_ptr<Material> material() const;

        glm::vec3 get_transform_center() const;
		bool test(glm::vec3 camera, const Frustum& frustum) const;

        void set_transform(const glm::mat4& tr);
        const glm::mat4& transform() const;

        SceneObject genlod(const GPULODGen& lod, float cellsize) const;

        SceneObjectUploaded upload() const;

        static SceneObject mk_aabb(const BoundingCriteria& box, glm::vec3 color);

    friend class SceneObjectUploaded;
    friend class SceneryMerger;
};

class SceneObjectUploaded : NonCopyable {
    private:
        glm::mat4 _transform = glm::mat4(1.0f);

        std::shared_ptr<StaticMeshUploaded> _mesh;
        std::shared_ptr<Material> _material;

    public:
        SceneObjectUploaded(const SceneObject&);

		bool test(glm::vec3 camera, const Frustum& frustum) const;
        void render(bool shade) const;

        void set_transform(const glm::mat4& tr);
        const glm::mat4& transform() const;

        const SceneObjectUploaded genlod(const GPULODGen& lod, float cellsize) const;
    
    friend class SceneObject;
};

}

#endif // SCENEOBJECT_H
