const int MaxLightCount = 8;
const int MaxCascadeCount = 4;

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform Lighting
{
	vec3 light_col[MaxLightCount];
	vec4 light_pos[MaxLightCount]; // xyz: position, w: inverse square radius
	vec4 light_dir[MaxLightCount]; // xyz: direction, w: spot light angle

	mat4x4 shadow_mats[MaxCascadeCount];
	float shadow_splits[MaxCascadeCount + 1];

	mat4x4 perspective_mat;
    vec3 ambient_color;
    vec2 half_near_plane;
	vec2 shadow_map_scale;
	vec2 frame_resolution;

	int point_count;
	int spot_count;
	int shadow_casc_count;

	float shadow_bias_offset;
	float shadow_bias_factor;
	float shadow_bias_clamp;
};
