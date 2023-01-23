#ifndef MATERIAL_H
#define MATERIAL_H

#include <Program.h>
#include <Texture.h>

#include <memory>
#include <vector>

namespace OM3D {

enum class BlendMode {
    None,
    Alpha,
};

enum class DepthTestMode {
    Standard,
    Reversed,
    Equal,
    None
};

enum CullingMode {
	None,
	Back,
	Front,
	FrontAndBack,
};

class Material {

    public:
        Material();

        bool is_instanced() const;

        void set_program(std::shared_ptr<Program> prog);
        void set_blend_mode(BlendMode blend);
        void set_depth_test_mode(DepthTestMode depth);
        void set_cull_mode(CullingMode cull);
        void set_texture(u32 slot, std::shared_ptr<Texture> tex);

        template<typename... Args>
        void set_uniform(Args&&... args) {
            _program->set_uniform(FWD(args)...);
        }


        void bind() const;

        std::size_t hash() const;
        bool operator==(const Material& other) const;

        static std::shared_ptr<Material> empty_material(bool instanced = false);
        static Material textured_material(bool instanced = false);
        static Material textured_normal_mapped_material(bool instanced = false);


    private:
        bool instanced;
        std::shared_ptr<Program> _program;
        std::vector<std::pair<u32, std::shared_ptr<Texture>>> _textures;

        BlendMode _blend_mode = BlendMode::None;
        DepthTestMode _depth_test_mode = DepthTestMode::Standard;
		CullingMode _cull_mode = CullingMode::Back;

};

}

namespace std {
    template<> struct hash<OM3D::Material> {
        inline std::size_t operator()(const OM3D::Material& m) const {
            return m.hash();
        }
    };
}

#endif // MATERIAL_H
