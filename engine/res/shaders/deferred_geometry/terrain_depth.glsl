#version 450
#property height_map tex2d

#stage vertex
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/deferred_geometry/terrain_uniform.glsl"

layout(location = VERTEX_ATTR_INDEX_POS) in vec2 position;

uniform sampler2D height_map;

vec4 calc_position()
{
	const float texel_size = 1.0 / (uniforms.terrain_side_verts + 2);
	const vec2 origin = vec2(1.5) * texel_size;
	const float border_scale_factor = (uniforms.terrain_side_verts - 1) / (uniforms.terrain_side_verts + 2);
	vec2 tex_coord = origin + position * border_scale_factor;
	float height_sample = texture(height_map, tex_coord).r;
	vec2 xy_pos = (position + uniforms.tile_offset) * uniforms.terrain_size * uniforms.tile_scale;
	return vec4(xy_pos.x, uniforms.height_origin + height_sample * uniforms.height_range, xy_pos.y, 1.0);
}

void main()
{
	gl_Position = uniforms.MVP * calc_position();
}

#stage fragment

void main()
{
}
