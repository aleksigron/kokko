#version 450
#property albedo_map tex2d
#property normal_map tex2d
#property roughness_map tex2d
#property color_tint vec3
#property metalness float
#property roughness float

#stage vertex
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/transform_block.glsl"

layout(location = VERTEX_ATTR_INDEX_POS) in vec3 position;
layout(location = VERTEX_ATTR_INDEX_NOR) in vec3 normal;
layout(location = VERTEX_ATTR_INDEX_TAN) in vec3 tangent;
layout(location = VERTEX_ATTR_INDEX_UV0) in vec2 tex_coord;

out VS_TO_FS {
	vec2 tex_coord;
	mat3 TBN;
} vs_out;

void main()
{
	vec3 N = normalize(vec3(transform.MV * vec4(normal, 0.0)));
	vec3 T = normalize(vec3(transform.MV * vec4(tangent, 0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	gl_Position = transform.MVP * vec4(position, 1.0);
	vs_out.tex_coord = tex_coord;
	vs_out.TBN = mat3(T, B, N);
}

#stage fragment
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/g_buffer_io.glsl"
#include "engine/shaders/common/deferred_frag_output.glsl"

in VS_TO_FS {
	vec2 tex_coord;
	mat3 TBN;
} fs_in;

uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D roughness_map;

void main()
{
	vec3 tan_normal = texture(normal_map, fs_in.tex_coord).rgb * 2.0 - 1.0;
	vec3 v_normal = normalize(fs_in.TBN * tan_normal);
	float tex_roughness = texture(roughness_map, fs_in.tex_coord).r;

	g_albedo = texture(albedo_map, fs_in.tex_coord).rgb * color_tint;
	g_normal = pack_normal(v_normal);
	g_material = vec3(metalness, tex_roughness * roughness, 0.0);
}
