const int MaxLightCount = 8;
const int MaxCascadeCount = 4;

layout(std140, binding = 0) uniform Lighting
{
	layout(offset = 0) int point_count;
	layout(offset = 4) int spot_count;
	layout(offset = 8) int shd_casc_count;

    layout(offset = 16) vec2 half_near_plane;

	layout(offset = 32) mat4x4 pers_mat;
	
	layout(offset = 96) vec3 light_col[MaxLightCount];
	layout(offset = 224) vec3 light_pos[MaxLightCount];
	layout(offset = 352) vec3 light_dir[MaxLightCount];
	layout(offset = 480) float light_angle[MaxLightCount];

	layout(offset = 608) mat4x4 shd_mat[MaxCascadeCount];
	layout(offset = 864) float shd_splits[MaxCascadeCount + 1];
} un;
