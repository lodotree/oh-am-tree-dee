#pragma once

#include <StaticMesh.h>
#include <SceneObject.h>
#include <SceneneryMerger.h>
#include <GPULODGen.h>

#include <variant>

namespace OM3D {

class Chunk;
class ChunkUploaded;
class ChunkyScene;
class ChunkySceneUploaded;

constexpr std::size_t MINSUBCHUNK = 8;
constexpr std::size_t CHUNKDLODS = 2;
constexpr std::array<float, CHUNKDLODS> SUBCHUNKSLODSUBDIVSM{{8.0, 16.0}};
constexpr std::array<float, CHUNKDLODS> SUBCHUNKSLODSUBDIVS{{4.0, 7.5}};

class Chunk {
	private:
		BoundingCriteria bb;
		std::variant<std::vector<Chunk>, SceneObject> fullres;
		std::array<SceneObject, CHUNKDLODS> lods;
		glm::vec3 gravicenter;
	public:
		template<typename It> Chunk(It begin, It end){
			// 1. Find clusters
			// 2. Generate a chunk for each cluster [recurse]
			// 3. Generate LOD for self
			const auto ocunt = end-begin;
			//1.
			//let's use k-d tree clustering based on centers
			//except i'm too lazy to compute actual median :(
			glm::vec3 middle;
			for(auto oo = begin; oo != end; oo++){
				const SceneObject& o = *oo;
				middle += (o.get_transform_center() + o.mesh().bb().center())/float(ocunt);
			}
			gravicenter = middle;
			std::array<std::vector<std::reference_wrapper<const SceneObject>>, 8> kd;
			for(auto oo = begin; oo != end; oo++){
				const SceneObject& o = *oo;
				const auto bbc = o.get_transform_center() + o.mesh().bb().center();
				kd[(bbc.x > middle.x ? 0b100 : 0) | (bbc.y > middle.y ? 0b010 : 0) | (bbc.z > middle.z ? 0b001 : 0)].push_back(o);
			}
			if(ocunt >= MINSUBCHUNK){
				//2.
				std::vector<Chunk> subchunks;
				for(auto& cluster : kd) if(cluster.size() > 0){
					subchunks.push_back(Chunk(cluster.begin(), cluster.end()));
				}
				//3.
				GPULODGen lodg;
				const SceneObject& beg = *begin;
				SceneryMerger merger(beg.material());
				for(const auto& c : subchunks) if(const auto* fr = std::get_if<SceneObject>(&c.fullres)) merger.add(*fr); else merger.add(c.lods[CHUNKDLODS-1]);
				auto fr = merger.morge();
				for(std::size_t i = 0; i < CHUNKDLODS; i++) lods[i] = fr.genlod(lodg, -SUBCHUNKSLODSUBDIVS[i]);
				//up.
				fullres = std::move(subchunks);
				bb = fr.mesh().bb();
			} else {
				//2.-
				const SceneObject& beg = *begin;
				SceneryMerger merger(beg.material());
				for(auto o = begin; o != end; o++) merger.add(*o);
				auto fr = merger.morge(); //TODO this relies on the merger behaviour aligning everything to the global center
				//3.
				GPULODGen lodg;
				for(std::size_t i = 0; i < CHUNKDLODS; i++) lods[i] = fr.genlod(lodg, -SUBCHUNKSLODSUBDIVSM[i]);
				//up.
				fullres = std::move(fr);
				bb = fr.mesh().bb();
			}
		}
		ChunkUploaded upload() const;
	
	friend class ChunkUploaded;
};
class ChunkUploaded {
	private:
		BoundingCriteria bb;
		std::variant<std::vector<ChunkUploaded>, SceneObjectUploaded> fullres;
		std::array<SceneObjectUploaded, CHUNKDLODS> self; //2 LOD levels then subchunking
		glm::vec3 gravicenter;

		ChunkUploaded(BoundingCriteria, std::variant<std::vector<ChunkUploaded>, SceneObjectUploaded>&&, std::array<SceneObjectUploaded, CHUNKDLODS>&&, glm::vec3);
	public:		
		bool test(glm::vec3 camera, const Frustum& frustum) const;
        void render(const glm::mat4& view_proj, glm::vec3 camera, const Frustum& frustum, bool shade, bool fullres) const;

		static bool DEBUG_CLUSTER_STATUS;
	friend class Chunk;
};

class ChunkyScene {};
class ChunkySceneUploaded {};

}
