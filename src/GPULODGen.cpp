#include "GPULODGen.h"

#include "TypedBuffer.h"
#include "shader_structs.h"

#include <vector>
#include <iostream>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace OM3D {

GPULODGen::GPULODGen() : programs {{
		Program::from_file("lod-1.comp"),
		Program::from_file("lod-2.comp"),
		Program::from_file("lod-3.comp"),
	}} {
	//
}

template<typename T> std::vector<T> download(const TypedBuffer<T>& buff, size_t count=0){
	auto view = buff.map(AccessType::ReadOnly);
	return std::vector(view.data(), view.data()+(count > 0 ? count : view.element_count()));
}

template<typename T> TypedBuffer<T> alloc(auto size){
	auto buf = TypedBuffer((const T*) nullptr, size);
	buf.clear();
	return buf;
}

StaticMesh GPULODGen::gen_lod(const StaticMesh& mesh, float cellsize) const {
	const auto delta = glm::vec3(1E-5, 1E-5, 1E-5);
	const auto aabb_aa = mesh.bb()._aa-delta;
	const auto aabb_bb = mesh.bb()._bb+delta;
	if(cellsize < 0){
		const auto subdiv = -cellsize;
		const auto ds = (aabb_bb - aabb_aa);
		cellsize = std::min(ds.x/subdiv, std::min(ds.y/subdiv, ds.z/subdiv));
	}
	const auto cellz = glm::uvec3(glm::ceil((aabb_bb - aabb_aa)/cellsize));
	const auto C = cellz.x * cellz.y * cellz.z;
	const glm::uint V = mesh.data().vertices.size();
	const glm::uint I = mesh.data().indices.size();
	const auto lod = shader::LODData {
		V, I,
		aabb_aa, aabb_bb,
		cellsize, cellz,
		fake ? 0 : 1,
	};
	auto in_loddata = TypedBuffer(Span(lod));
	const auto set_loddata = [&](){
		in_loddata.bind(BufferUsage::Storage, 39);
	};
	std::cout << "# Beginning LOD generation #" << std::endl;
	std::cout << "Mesh [" << V << "," << I << "] => " << cellz.x << "," << cellz.y << "," << cellz.z << " / " << C << " cells" << std::endl;
	/*{
		const auto vs = download(mesh._vertex_buffer);
		std::vector<u32> cekk(V);
		std::cout << "expected qpos" << std::endl;
		for(size_t i = 0; i < V; i++){
			const auto p = vs[i].position;
			std::cout << p.x << "," << p.y << "," << p.z << ", ";
		}
		std::cout << std::endl;
		std::cout << "expected v2c" << std::endl;
		for(size_t i = 0; i < V; i++){
			const auto pos = vs[i].position;
			glm::vec3 rfpos = (pos - lod.aabb_aa)/(lod.aabb_bb-lod.aabb_aa);
			glm::uvec3 cp = glm::uvec3(glm::floor(rfpos*glm::vec3(lod.cellz)));
			// std::cout << "{" << cp.x << "," << cp.y << "," << cp.z << "}, ";
			std::cout << (cp.x*lod.cellz.x + cp.y)*cellz.y + cp.z << ", ";
		}
		std::cout << std::endl;
	}*/
	// uvec3 cellz = std::ceil((mesh.b))
	// First pass - generate cells

	/// old vertex id -> old vertex data
	TypedBuffer<Vertex> in_vertices(mesh.data().vertices);
	/// old vertex index id -> old vertex id
	TypedBuffer<u32> in_indices(mesh.data().indices);
	/// cell id -> quadric
	auto inout_qem = alloc<shader::QuadricCellInfo>(C);
	/// old vertex id -> cell id
	auto inout_cellidx = alloc<glm::uint>(V);
	/// old vertex id -> position
	auto inout_qpos = alloc<glm::vec4>(V);
	{
		programs[0]->bind();
		set_loddata();
		in_vertices.bind(BufferUsage::Storage, 0);
		in_indices.bind(BufferUsage::Storage, 1);
		inout_qem.bind(BufferUsage::Storage, 2);
		inout_cellidx.bind(BufferUsage::Storage, 3);
		inout_qpos.bind(BufferUsage::Storage, 4);
		glDispatchCompute(align_up_to(V, 1024)/1024, 1, 1);
		glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT|GL_SHADER_STORAGE_BARRIER_BIT);
	}
	std::cout << "1st pass sucessful" << std::endl;
	/*{
		auto v2c = download(inout_cellidx);
		std::cout << v2c.size() << " v2c entries" << std::endl;
		for(auto& v : v2c) std::cout << v << ", ";
		std::cout << std::endl;
		auto cells = download(inout_qem);
		std::cout << cells.size() << " cells" << std::endl;
		for(auto& c : cells) std::cout << c.count << ", ";
		std::cout << std::endl;
		auto qpos = download(inout_qpos);
		std::cout << qpos.size() << " qpos" << std::endl;
		for(auto& p : qpos) std::cout << p.x << "," << p.y << "," << p.z << ", ";
		std::cout << std::endl;
	}*/
	// Second pass
	const glm::uint inout_vcount_data = 0;
	/// number of new vertices
	auto inout_vcount = TypedBuffer(Span(inout_vcount_data));
	/// cell id -> new vertex id
	auto inout_vidx = alloc<glm::uint>(C);
	/// new vertex id -> vertex data
	auto inout_vertices = alloc<Vertex>(C);
	{
		programs[1]->bind();
		set_loddata();
		inout_qem.bind(BufferUsage::Storage, 0);
		inout_vcount.bind(BufferUsage::Storage, 1);
		inout_vidx.bind(BufferUsage::Storage, 2);
		inout_vertices.bind(BufferUsage::Storage, 3);
		glDispatchCompute(align_up_to(C, 1024)/1024, 1, 1);
		glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT|GL_SHADER_STORAGE_BARRIER_BIT);
	}
	const auto V_ = ([&](){
		auto buf = inout_vcount.map(AccessType::ReadOnly);
		return buf[0];
	})();
	std::cout << "Second pass successful, new vertices = " << V_ << std::endl;
	// Third pass
	const glm::uint out_vcount_data = 0;
	/// number of spwnd vertices
	auto out_vcount = TypedBuffer(Span(out_vcount_data));
	/// spwnd vertex id -> spwnd vertex data
	auto out_vertices = alloc<Vertex>(std::max(C, V));
	const glm::uint inout_tricount_data = 0;
	/// number of spwnd triangles
	auto out_tricount = TypedBuffer(Span(inout_tricount_data));
	/// spwnd vertex index id -> spwnd vertex id
	auto out_vertindices = alloc<glm::uint>(std::max(C, I));
	{
		programs[2]->bind();
		set_loddata();
		in_vertices.bind(BufferUsage::Storage, 0);
		in_indices.bind(BufferUsage::Storage, 1);
		inout_cellidx.bind(BufferUsage::Storage, 2);
		inout_qem.bind(BufferUsage::Storage, 3);
		inout_vidx.bind(BufferUsage::Storage, 4);
		inout_vcount.bind(BufferUsage::Storage, 5);
		inout_vertices.bind(BufferUsage::Storage, 6);
		out_vcount.bind(BufferUsage::Storage, 7);
		out_vertices.bind(BufferUsage::Storage, 8);
		out_tricount.bind(BufferUsage::Storage, 9);
		out_vertindices.bind(BufferUsage::Storage, 10);
		glDispatchCompute(align_up_to(I/3, 1024)/1024, 1, 1);
		glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT|GL_SHADER_STORAGE_BARRIER_BIT);
	}
	const auto V__ = download(out_vcount)[0];
	std::cout << "Third pass successful, final vertices = " << V__ << std::endl;
	const auto tri_ = ([&](){
		auto buf = out_tricount.map(AccessType::ReadOnly);
		return buf[0];
	})();
	const auto I_ = tri_ * 3;
	std::cout << "Third pass successful, new indices = " << I_ << std::endl;
	// Download
	const auto vertices = download(out_vertices, V__);
	const auto vertindices = download(out_vertindices, I_);
	return MeshData { vertices, vertindices, };
}

}
