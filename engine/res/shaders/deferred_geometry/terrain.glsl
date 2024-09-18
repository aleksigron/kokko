#version 450
#property height_map tex2d
#property albedo_map tex2d

#stage vertex
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/deferred_geometry/terrain_uniform.glsl"

layout(location = VERTEX_ATTR_INDEX_POS) in vec2 position;

out VS_TO_FS {
	vec3 normal;
	vec2 tex_coord;
} vs_out;

uniform sampler2D height_map;

vec3 calc_position(vec2 offset)
{
	const float texel_size = 1.0 / (uniforms.terrain_side_verts + 2);
	const vec2 origin = vec2(1.5) * texel_size;
	const float border_scale_factor = (uniforms.terrain_side_verts - 1) / (uniforms.terrain_side_verts + 2);
	vec2 pos_tile_space = position + offset;
	vec2 tex_coord = origin + pos_tile_space * border_scale_factor;
	float height_sample = texture(height_map, tex_coord).r;
	vec2 xy_pos = (pos_tile_space + uniforms.tile_offset) * uniforms.terrain_size * uniforms.tile_scale;
	return vec3(xy_pos.x, uniforms.height_origin + height_sample * uniforms.height_range, xy_pos.y);
}

void main()
{
	float offset_amount = 1.0 / (uniforms.terrain_side_verts - 1);

	vec3 p_0 = calc_position(vec2(0.0, 0.0));
	vec3 p_right = calc_position(vec2(offset_amount, 0.0));
	vec3 p_left = calc_position(vec2(-offset_amount, 0.0));
	vec3 p_top = calc_position(vec2(0.0, offset_amount));
	vec3 p_bottom = calc_position(vec2(0.0, -offset_amount));

	vec3 x_tan = normalize(p_right - p_left);
	vec3 z_tan = normalize(p_bottom - p_top);
	vec3 w_normal = cross(x_tan, z_tan);

	gl_Position = uniforms.MVP * vec4(p_0, 1.0);

	vs_out.normal = normalize(vec3(uniforms.MV * vec4(w_normal, 0.0)));
	vs_out.tex_coord = p_0.xz * uniforms.texture_scale;
}

#stage fragment
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/deferred_frag_output.glsl"
#include "engine/shaders/common/g_buffer_io.glsl"
#include "engine/shaders/deferred_geometry/terrain_uniform.glsl"

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
	g_material = vec3(uniforms.metalness, uniforms.roughness, 0.0);
}
