#pragma once

#include <graphics.h>
#include <Vertex.h>

#include <Program.h>
#include <SceneObject.h>

#include <vector>
#include <array>
#include <functional>

namespace OM3D {

class SceneryMerger : NonCopyable {
	private:
		std::shared_ptr<Material> material;
		std::vector<std::reference_wrapper<const SceneObject>> objects;
	public:
		SceneryMerger(std::shared_ptr<Material>);
		SceneryMerger& add(const SceneObject& object);
		SceneObject morge();
};

}
