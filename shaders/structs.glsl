// #ifndef __GLSL__ALIGN__
// #define __GLSL__ALIGN__
// #endif

struct CameraData {
    mat4 view_proj;
    mat4 inv_view_proj;
};

struct FrameData {
    CameraData camera;

    vec3 sun_dir;
    uint point_light_count;

    vec3 sun_color;
    float padding_1;
};

struct PointLight {
    vec3 position;
    float radius;
    vec3 color;
    float padding_1;
};

#ifdef _GLSL_CPP_
struct LODData {
    alignas(sizeof(vec4)) uint v_count; //V
	alignas(sizeof(vec4)) uint vi_count;//I
	/// minimal corner
	alignas(sizeof(vec4)) vec3 aabb_aa;
	/// maximal corner
	alignas(sizeof(vec4)) vec3 aabb_bb;
	/// Size of a cell
	alignas(sizeof(vec4)) float abs_cellsize; // C := ceil((aabb_bb-aabb_aa).x/abs_cellsize) * ceil((aabb_bb-aabb_aa).y/abs_cellsize) * ceil((aabb_bb-aabb_aa).z/abs_cellsize)
    alignas(sizeof(vec4)) uvec3 cellz; // ceil((aabb_bb-aabb_aa)/abs_cellsize) 
    alignas(sizeof(vec4)) uint mode;
};
#else
struct LODData {
	uint v_count; //V
    float _pad1[3];
	uint vi_count;//I
    float _pad2[3];
	/// minimal corner
	vec3 aabb_aa;
    float _pad3[1];
	/// maximal corner
	vec3 aabb_bb;
    float _pad4[1];
	/// Size of a cell
	float abs_cellsize; // C := ceil((aabb_bb-aabb_aa).x/abs_cellsize) * ceil((aabb_bb-aabb_aa).y/abs_cellsize) * ceil((aabb_bb-aabb_aa).z/abs_cellsize)
    float _pad5[3];
    uvec3 cellz; // ceil((aabb_bb-aabb_aa)/abs_cellsize) 
    float _pad6[1];
    uint mode;
    float _pad7[3];
};
#endif


#ifdef _GLSL_CPP_
// struct VertexDataUnaligned {
// 	alignas(16) vec3 position;
// 	alignas(16) vec3 normal;
// 	alignas(16) vec2 uv;
// 	alignas(16) vec4 tangent;
// 	alignas(16) vec3 color;
// };
// #define ROUND_DOWN(N,S) ((N / S) * S)
// #define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))
// // struct VertexData {
// // 	alignas(16) vec3 position;
// // 	alignas(16) vec3 normal;
// // 	alignas(16) vec2 uv;
// // 	alignas(16) vec4 tangent;
// // 	alignas(16) vec3 color;
// //     float __alignment[(ROUND_UP(sizeof(VertexDataUnaligned), sizeof(vec4)) - sizeof(VertexDataUnaligned))/sizeof(float)];
// // };
// struct VertexData {
// 	vec4 position;
//     // float _pad1[1];
// 	// vec4 normal;
//     // float _pad2[1];
// 	// vec2 uv;
//     // float _pad3[2];
// 	// vec4 tangent;
// 	// vec3 color;
//     // float _pad4[1];
// };
#else
struct VertexData {
	vec3 position;
    float _pad1[1];
	vec3 normal;
    float _pad2[1];
	vec2 uv;
    float _pad3[2];
	vec4 tangent;
	vec3 color;
    float _pad4[1];
};
#endif

/*
struct FundamentalQuadric {
	mat3 A;
	vec3 b;
	float c;
};
// aka mat4
*/

#ifdef _GLSL_CPP_
struct QuadricCellInfo {
    alignas(sizeof(vec4)) uint count;
    alignas(sizeof(vec4)) vec3 sumpos;
    alignas(sizeof(vec4)) vec3 sumnorm;
    alignas(sizeof(vec4)) vec3 sumcolor;
    alignas(sizeof(vec4)) mat4 quadric; //FundamentalQuadric
};
#else
struct QuadricCellInfo {
    uint count;
    float _pad1[3];
    vec3 sumpos;
    float _pad2[1];
    vec3 sumnorm;
    float _pad3[1];
    vec3 sumcolor;
    float _pad4[1];
    mat4 quadric; //FundamentalQuadric
};
#endif
