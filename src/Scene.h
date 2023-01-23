#ifndef SCENE_H
#define SCENE_H

#include <SceneObject.h>
#include <PointLight.h>
#include <Camera.h>

#include <ChunkyScene.h>

#include <vector>
#include <memory>

namespace OM3D {

class Scene;
class SceneUploaded;

class Scene {
    private:
        std::vector<SceneObject> _objects;
        std::vector<Chunk> _chunks;
        std::vector<PointLight> _point_lights;
        glm::vec3 _sun_direction = glm::vec3(0.2f, 1.0f, 0.1f);
    
    public:
        Scene();

        static Result<Scene> from_gltf(const std::string& file_name, bool chonk);

        void add_object(SceneObject obj);
        void add_object(Chunk obj);
        void add_object(PointLight obj);

        Scene genlod(const GPULODGen& lod, float cellsize) const;

        std::unique_ptr<SceneUploaded> upload() const;

    friend class SceneUploaded;
};

class SceneUploaded : NonMovable {
    private:
        std::vector<SceneObjectUploaded> _objects;
        std::vector<ChunkUploaded> _chunks;
        std::vector<PointLight> _point_lights;
        glm::vec3 _sun_direction = glm::vec3(0.2f, 1.0f, 0.1f);

    public:
        SceneUploaded(const Scene&);

		void bind_light_buffer(const Camera& camera, u32 idx=1) const;
        void render(const Camera& camera, bool shade, bool chunks_force_fullres) const;
    
    friend class Scene;
};

}

#endif // SCENE_H
