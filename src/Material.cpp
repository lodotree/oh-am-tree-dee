#include "Material.h"

#include <glad/glad.h>

#include <algorithm>

namespace OM3D {

Material::Material() {
}

bool Material::is_instanced() const {
    return instanced;
}

void Material::set_program(std::shared_ptr<Program> prog) {
    _program = std::move(prog);
}

void Material::set_blend_mode(BlendMode blend) {
    _blend_mode = blend;
}

void Material::set_depth_test_mode(DepthTestMode depth) {
    _depth_test_mode = depth;
}

void Material::set_cull_mode(CullingMode cull) {
    _cull_mode = cull;
}

void Material::set_texture(u32 slot, std::shared_ptr<Texture> tex) {
    if(const auto it = std::find_if(_textures.begin(), _textures.end(), [&](const auto& t) { return t.second == tex; }); it != _textures.end()) {
        it->second = std::move(tex);
    } else {
        _textures.emplace_back(slot, std::move(tex));
    }
}

void Material::bind() const {
    switch(_blend_mode) {
        case BlendMode::None:
            glDisable(GL_BLEND);
        break;

        case BlendMode::Alpha:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    }

    switch(_depth_test_mode) {
        case DepthTestMode::None:
            glDisable(GL_DEPTH_TEST);
        break;

        case DepthTestMode::Equal:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
        break;

        case DepthTestMode::Standard:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_GEQUAL);
        break;

        case DepthTestMode::Reversed:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_LEQUAL);
        break;
    }

	switch(_cull_mode){
		case None:
			glDisable(GL_CULL_FACE);
		break;
		case Back:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		break;
		case Front:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
		break;
		case FrontAndBack:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT_AND_BACK);
		break;
	}

    for(const auto& texture : _textures) {
        texture.second->bind(texture.first);
    }
    _program->bind();
}

std::size_t Material::hash() const {
    auto h = _program->hash();
    for(const auto& t : _textures) h ^= t.second->hash();
    return h;
}
bool Material::operator==(const Material& other) const {
    return _program == other._program && _textures == other._textures;
}

std::shared_ptr<Material> Material::empty_material(bool instanced) {
    static std::weak_ptr<Material> weak_material;
    auto material = weak_material.lock();
    if(!material) {
        material = std::make_shared<Material>();
        material->instanced = instanced;
        material->_program = Program::from_files("lit.frag", instanced ? "basic_i.vert" : "basic.vert");
        weak_material = material;
    }
    return material;
}

Material Material::textured_material(bool instanced) {
    Material material;
    material.instanced = instanced;
    material._program = Program::from_files("lit.frag", instanced ? "basic_i.vert" : "basic.vert", {"TEXTURED"});
    return material;
}

Material Material::textured_normal_mapped_material(bool instanced) {
    Material material;
    material.instanced = instanced;
    material._program = Program::from_files("lit.frag", instanced ? "basic_i.vert" : "basic.vert", std::array<std::string, 2>{"TEXTURED", "NORMAL_MAPPED"});
    return material;
}


}
