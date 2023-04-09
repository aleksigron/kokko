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
	float height_origin;
	float height_range;
	float metalness;
	float roughness;
}
uniforms;
