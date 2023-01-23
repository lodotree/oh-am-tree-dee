#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <Scene.h>
#include <Camera.h>

namespace OM3D {

class SceneView {
    public:
        SceneView(const SceneUploaded* scene = nullptr);

        Camera& camera();
        const Camera& camera() const;

		void bind_lights(u32 idx = 10) const;
        void render(bool shade, bool chunks_force_fullres) const;

        SceneView transfer(const SceneUploaded* newscene) const;

    private:
        const SceneUploaded* _scene = nullptr;
        std::shared_ptr<Camera> _camera;

};

}

#endif // SCENEVIEW_H
