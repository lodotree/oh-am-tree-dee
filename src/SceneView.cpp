#include "SceneView.h"

namespace OM3D {

SceneView::SceneView(const SceneUploaded* scene) : _scene(scene) {
    _camera = std::make_shared<Camera>(std::move(Camera()));
}

Camera& SceneView::camera() {
    return *_camera;
}

const Camera& SceneView::camera() const {
    return *_camera;
}

void SceneView::bind_lights(u32 idx) const {
    if(_scene) {
        _scene->bind_light_buffer(*_camera, idx);
    }
}

void SceneView::render(bool shade, bool chunks_force_fullres) const {
    if(_scene) {
        _scene->render(*_camera, shade, chunks_force_fullres);
    }
}

SceneView SceneView::transfer(const SceneUploaded* newscene) const {
    auto s = SceneView(newscene);
    s._camera = _camera;
    return s;
}

}
