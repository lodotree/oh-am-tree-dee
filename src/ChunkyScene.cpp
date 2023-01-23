#include "ChunkyScene.h"

#include <algorithm>

namespace OM3D {

ChunkUploaded Chunk::upload() const {
	if(const auto* subchunks = std::get_if<std::vector<Chunk>>(&fullres)){
		std::vector<ChunkUploaded> subu;
		std::transform(subchunks->begin(), subchunks->end(), std::back_inserter(subu), [](const auto& c){ return c.upload(); });
		return ChunkUploaded{
			bb,
			std::move(subu),
			{lods[0].upload(), lods[1].upload()},
			gravicenter,
		};
	} else {
		return ChunkUploaded{
			bb,
			std::move(std::get_if<SceneObject>(&fullres)->upload()),
			{lods[0].upload(), lods[1].upload()},
			gravicenter,
		};
	}
}

ChunkUploaded::ChunkUploaded(BoundingCriteria _bb, std::variant<std::vector<ChunkUploaded>, SceneObjectUploaded>&& _fr, std::array<SceneObjectUploaded, CHUNKDLODS>&& _lods, glm::vec3 _gc) : bb(_bb), fullres(std::move(_fr)), self(std::move(_lods)), gravicenter(_gc) {}

bool ChunkUploaded::test(glm::vec3 camera, const Frustum& frustum) const {
	return bb.test(glm::vec3(0.0f, 0.0f, 0.0f), camera, frustum);
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

bool ChunkUploaded::DEBUG_CLUSTER_STATUS = false;

void ChunkUploaded::render(const glm::mat4& view_proj, glm::vec3 camera, const Frustum& frustum, bool shade, bool force_fullres) const {
	const auto d2c = glm::distance(gravicenter, camera);
	/*
	const auto trab = bb.transform_aabb(view_proj);
	constexpr float pixw = 1920, pixh = 1080; //TODO do not hardcode :(
	auto pixelz = ((trab.bb().x - trab.aa().x)*pixw)*((trab.bb().y - trab.aa().y)*pixh);
	*/
	const auto d2cc = std::min(
		std::min(
		std::min(
		glm::distance(glm::vec3(bb.aa().x, bb.aa().y, bb.aa().z), camera),
		glm::distance(glm::vec3(bb.aa().x, bb.aa().y, bb.bb().z), camera)
		), std::min(
		glm::distance(glm::vec3(bb.aa().x, bb.bb().y, bb.aa().z), camera),
		glm::distance(glm::vec3(bb.aa().x, bb.bb().y, bb.bb().z), camera)
		)), std::min(std::min(
		glm::distance(glm::vec3(bb.bb().x, bb.aa().y, bb.aa().z), camera),
		glm::distance(glm::vec3(bb.bb().x, bb.aa().y, bb.bb().z), camera)
		), std::min(
		glm::distance(glm::vec3(bb.bb().x, bb.bb().y, bb.aa().z), camera),
		glm::distance(glm::vec3(bb.bb().x, bb.bb().y, bb.bb().z), camera)
		))
	);
	if(auto* fro = std::get_if<SceneObjectUploaded>(&fullres)){
		if(d2cc < 750 || force_fullres){
			if(DEBUG_CLUSTER_STATUS) SceneObject::mk_aabb(bb, glm::vec3(0.25, 0.25, 1.0)).upload().render(shade);
			fro->render(shade);
		} else if(d2cc < 2000){
			if(DEBUG_CLUSTER_STATUS) SceneObject::mk_aabb(bb, glm::vec3(0.25, 0.25, 0.75)).upload().render(shade);
			self[1].render(shade);
		} else {
			if(DEBUG_CLUSTER_STATUS) SceneObject::mk_aabb(bb, glm::vec3(0.25, 0.25, 0.5)).upload().render(shade);
			self[0].render(shade);
		}
	}
	else if(auto* subchunks = std::get_if<std::vector<ChunkUploaded>>(&fullres)){
		if(d2cc < 8000 || force_fullres){
			if(DEBUG_CLUSTER_STATUS) SceneObject::mk_aabb(bb, glm::vec3(0.25, 1.0, 0.25)).upload().render(shade);
			for(const auto& sc : *subchunks) if(sc.test(camera, frustum)) sc.render(view_proj, camera, frustum, shade, force_fullres);
		} else if(d2cc < 20000){
			if(DEBUG_CLUSTER_STATUS) SceneObject::mk_aabb(bb, glm::vec3(0.75, 0.25, 0.25)).upload().render(shade);
			self[1].render(shade);
		} else {
			if(DEBUG_CLUSTER_STATUS) SceneObject::mk_aabb(bb, glm::vec3(0.5, 0.25, 0.25)).upload().render(shade);
			self[0].render(shade);
		}
	}
}

}
