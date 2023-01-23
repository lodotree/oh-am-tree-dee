#include "SceneneryMerger.h"

#include <algorithm>

namespace OM3D {

SceneryMerger::SceneryMerger(std::shared_ptr<Material> mat) : material(mat) {}

SceneryMerger& SceneryMerger::add(const SceneObject& o){
	if(o._material == material) objects.push_back(std::cref(o));
	return *this;
}

SceneObject SceneryMerger::morge(){
	//TODO maybe don't align everythin to the global center?
	MeshData result;
	u32 ni = 0;
	for(auto oo : objects){
		const auto& o = oo.get();
		const auto& mesh = o._mesh;
		const auto& is = mesh.data().indices;
		const auto& vs = mesh.data().vertices;
		const auto& transform = o._transform;
		std::transform(vs.begin(), vs.end(), std::back_inserter(result.vertices), [&](Vertex v){
			v.position = glm::vec3(transform * glm::vec4(v.position, 1.0f));
			return v;
		});
		std::transform(is.begin(), is.end(), std::back_inserter(result.indices), [&](auto i){ return ni+i; });
		ni += mesh.data().vertices.size();
	}
	return SceneObject(StaticMesh(result), material);
}

}
