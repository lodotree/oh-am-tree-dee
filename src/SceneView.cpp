#include "SceneView.h"

namespace OM3D {

SceneView::SceneView(const Scene* scene) : _scene(scene) {
}

Camera& SceneView::camera() {
    return _camera;
}

const Camera& SceneView::camera() const {
    return _camera;
}

void SceneView::bind_lights(u32 idx) const {
    if(_scene) {
        _scene->bind_light_buffer(_camera, idx);
    }
}

void SceneView::render(bool shade) const {
    if(_scene) {
        _scene->render(_camera, shade);
    }
}

}
