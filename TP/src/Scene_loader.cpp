#include "Scene.h"
#include "StaticMesh.h"

#include <utils.h>

#include <iostream>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NOEXCEPTION
#include <tinygltf/tiny_gltf.h>

namespace OM3D {

static size_t component_count(int type) {
    switch(type) {
        case TINYGLTF_TYPE_SCALAR: return 1;
        case TINYGLTF_TYPE_VEC2: return 2;
        case TINYGLTF_TYPE_VEC3: return 3;
        case TINYGLTF_TYPE_VEC4: return 4;
        case TINYGLTF_TYPE_MAT2: return 4;
        case TINYGLTF_TYPE_MAT3: return 9;
        case TINYGLTF_TYPE_MAT4: return 16;
        default: return 0;
    }
}

static bool decode_attrib_buffer(const tinygltf::Model& gltf, const std::string& name, const tinygltf::Accessor& accessor, Span<Vertex> vertices) {
    const tinygltf::BufferView& buffer = gltf.bufferViews[accessor.bufferView];

    if(accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
        std::cerr << "Unsupported component type (" << accessor.componentType << ") for \"" << name << "\"" << std::endl;
        return false;
    }

    const size_t vertex_count = vertices.size();

    auto decode_attribs =  [&](auto* vertex_elems) {
        using attrib_type = std::remove_reference_t<decltype(vertex_elems[0])>;
        using value_type = typename attrib_type::value_type;
        static constexpr size_t size = sizeof(attrib_type) / sizeof(value_type);

        const size_t components = component_count(accessor.type);
        const bool normalize = accessor.normalized;

        DEBUG_ASSERT(accessor.count == vertex_count);

        if(components != size) {
            std::cerr << "Expected VEC" << size << " attribute, got VEC" << components << std::endl;
        }

        const size_t min_size = std::min(size, components);
        auto convert = [=](const u8* data) {
            attrib_type vec;
            for(size_t i = 0; i != min_size; ++i) {
                vec[int(i)] = reinterpret_cast<const value_type*>(data)[i];
            }
            if(normalize) {
                if constexpr(size == 4) {
                    const glm::vec3 n = glm::normalize(glm::vec3(vec));
                    vec[0] = n[0];
                    vec[1] = n[1];
                    vec[2] = n[2];
                } else {
                    vec = glm::normalize(vec);
                }
            }
            return vec;
        };

        {
            u8* out_begin = reinterpret_cast<u8*>(vertex_elems);

            const auto& in_buffer = gltf.buffers[buffer.buffer].data;
            const u8* in_begin = in_buffer.data() + buffer.byteOffset + accessor.byteOffset;
            const size_t attrib_size = components * sizeof(value_type);
            const size_t input_stride = buffer.byteStride ? buffer.byteStride : attrib_size;

            for(size_t i = 0; i != accessor.count; ++i) {
                const u8* attrib = in_begin + i * input_stride;
                DEBUG_ASSERT(attrib < in_buffer.data() + in_buffer.size());
                *reinterpret_cast<attrib_type*>(out_begin + i * sizeof(Vertex)) = convert(attrib);
            }
        }
    };

    if(name == "POSITION") {
        decode_attribs(&vertices[0].position);
    } else if(name == "NORMAL") {
        decode_attribs(&vertices[0].normal);
    }/* else if(name == "TANGENT") {
        decode_attribs(&vertices[0].tangent);
    }*/ else if(name == "TEXCOORD_0") {
        decode_attribs(&vertices[0].uv);
    } else if(name == "COLOR_0") {
        decode_attribs(&vertices[0].color);
    } else {
        std::cerr << "Attribute \"" << name << "\" is not supported" << std::endl;
    }
    return true;
}

static bool decode_index_buffer(const tinygltf::Model& gltf, const tinygltf::Accessor& accessor, Span<u32> indices) {
    const tinygltf::BufferView& buffer = gltf.bufferViews[accessor.bufferView];

    auto decode_indices = [&](u32 elem_size, auto convert_index) {
        const u8* in_buffer = gltf.buffers[buffer.buffer].data.data() + buffer.byteOffset + accessor.byteOffset;
        const size_t input_stride = buffer.byteStride ? buffer.byteStride : elem_size;

        for(size_t i = 0; i != accessor.count; ++i) {
            indices[i] = convert_index(in_buffer + i * input_stride);
        }
    };

    switch(accessor.componentType) {
        case TINYGLTF_PARAMETER_TYPE_BYTE:
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
            decode_indices(1, [](const u8* data) -> u32 { return *data; });
        break;

        case TINYGLTF_PARAMETER_TYPE_SHORT:
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
            decode_indices(2, [](const u8* data) -> u32 { return *reinterpret_cast<const u16*>(data); });
        break;

        case TINYGLTF_PARAMETER_TYPE_INT:
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
            decode_indices(4, [](const u8* data) -> u32 { return *reinterpret_cast<const u32*>(data); });
        break;

        default:
            std::cerr << "Index component type not supported" << std::endl;
            return false;
    }

    return true;
}

static Result<MeshData> build_mesh_data(const tinygltf::Model& gltf, const tinygltf::Primitive& prim) {
    std::vector<Vertex> vertices;
    for(auto&& [name, id] : prim.attributes) {
        tinygltf::Accessor accessor = gltf.accessors[id];
        if(!accessor.count) {
            continue;
        }

        if(accessor.sparse.isSparse) {
            return {false, {}};
        }

        if(!vertices.size()) {
            std::fill_n(std::back_inserter(vertices), accessor.count, Vertex{});
        } else if(vertices.size() != accessor.count) {
            return {false, {}};
        }

        if(!decode_attrib_buffer(gltf, name, accessor, vertices)) {
            return {false, {}};
        }
    }


    std::vector<u32> indices;
    {
        tinygltf::Accessor accessor = gltf.accessors[prim.indices];
        if(!accessor.count || accessor.sparse.isSparse) {
            return {false, {}};
        }

        if(!indices.size()) {
            std::fill_n(std::back_inserter(indices), accessor.count, u32(0));
        } else if(indices.size() != accessor.count) {
            return {false, {}};
        }

        if(!decode_index_buffer(gltf, accessor, indices)) {
            return {false, {}};
        }
    }

    return {true, MeshData{std::move(vertices), std::move(indices)}};
}

Result<std::unique_ptr<Scene>> Scene::from_gltf(const std::string& file_name) {
    tinygltf::TinyGLTF ctx;
    tinygltf::Model gltf;

    {
        std::string err;
        std::string warn;

        const bool is_ascii = ends_with(file_name, ".gltf");
        const bool ok = is_ascii
                ? ctx.LoadASCIIFromFile(&gltf, &err, &warn, file_name)
                : ctx.LoadBinaryFromFile(&gltf, &err, &warn, file_name);

        if(!err.empty()) {
            std::cerr << "Error while loading gltf: " << err << std::endl;
        }
        if(!warn.empty()) {
            std::cerr << "Warning while loading gltf: " << warn << std::endl;
        }

        if(!ok) {
            return {false, {}};
        }
    }

    auto scene = std::make_unique<Scene>();

    for(int i = 0; i != gltf.meshes.size(); ++i) {
        const tinygltf::Mesh& mesh = gltf.meshes[i];

        for(int j = 0; j != mesh.primitives.size(); ++j) {
            const tinygltf::Primitive& prim = mesh.primitives[j];

            if(prim.mode != TINYGLTF_MODE_TRIANGLES) {
                continue;
            }

            const auto mesh = build_mesh_data(gltf, prim);
            if(!mesh.is_ok) {
                return {false, {}};
            }

            auto static_mesh = std::make_shared<StaticMesh>(mesh.value);
            scene->add_object(SceneObject(std::move(static_mesh), Material::empty_material()));
        }
    }

    return {true, std::move(scene)};
}

}
