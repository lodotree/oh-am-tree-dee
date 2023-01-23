#pragma once

#include <graphics.h>
#include <Vertex.h>

#include <Program.h>
#include <StaticMesh.h>

#include <vector>
#include <array>

namespace OM3D {

class GPULODGen : NonCopyable {
	private:
		std::array<std::shared_ptr<Program>, 3> programs;
	public:
		GPULODGen();
		StaticMesh gen_lod(const StaticMesh& mesh, float cellsize=-25.f) const;
		bool fake = false;
};

}
