#version 450
#property height_map tex2d
#property albedo_map tex2d
#property metalness float
#property roughness float

#stage vertex
#include "engine/shaders/common/constants.glsl"

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform TerrainBlock
{
	mat4x4 MVP;
	mat4x4 MV;
	vec2 texture_scale;
	vec2 tile_offset;
	float tile_scale;
	float terrain_size;
	float terrain_resolution;
	float min_height;
	float max_height;
}
uniforms;

layout(location = VERTEX_ATTR_INDEX_POS) in vec2 position;
layout(location = VERTEX_ATTR_INDEX_NOR) in vec3 normal;

out VS_TO_FS {
	vec3 normal;
	vec2 tex_coord;
} vs_out;

uniform sampler2D height_map;

float sample_height(vec2 offset)
{
	return texture(height_map, position + offset).r;
}

void main()
{
	float offset_amount = 1.0 / uniforms.terrain_resolution;
	float y_extent = uniforms.max_height - uniforms.min_height;
	float w_offset = uniforms.terrain_size / uniforms.terrain_resolution * 2.0;

	float h0 = sample_height(vec2(0.0, 0.0));
	float h1 = sample_height(vec2(offset_amount, 0.0));
	float h2 = sample_height(vec2(-offset_amount, 0.0));
	float h3 = sample_height(vec2(0.0, offset_amount));
	float h4 = sample_height(vec2(0.0, -offset_amount));

	float x_diff = (h2 - h1) * y_extent;
	float z_diff = (h4 - h3) * y_extent;
	
	float x_angle = atan(x_diff, w_offset);
	float z_angle = atan(z_diff, w_offset);

	vec3 x_tan = vec3(cos(x_angle), -sin(x_angle), 0.0);
	vec3 z_tan = vec3(0.0, -sin(z_angle), cos(z_angle));

	vec3 w_normal = cross(normalize(z_tan), normalize(x_tan));

	vec2 w_pos = (position + uniforms.tile_offset) * uniforms.terrain_size * uniforms.tile_scale;

	gl_Position = uniforms.MVP * vec4(w_pos.x, uniforms.min_height + h0 * y_extent, w_pos.y, 1.0);

	vs_out.normal = normalize(vec3(uniforms.MV * vec4(w_normal, 0.0)));
	vs_out.tex_coord = w_pos * uniforms.texture_scale;
}

#stage fragment
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/deferred_frag_output.glsl"
#include "engine/shaders/common/g_buffer_io.glsl"

in VS_TO_FS {
	vec3 normal;
	vec2 tex_coord;
} fs_in;

uniform sampler2D albedo_map;

void main()
{
	vec3 N = normalize(fs_in.normal);

	g_albedo = texture(albedo_map, fs_in.tex_coord).rgb;
	g_normal = pack_normal(N);
	g_material = vec3(metalness, roughness, 0.0);
}
