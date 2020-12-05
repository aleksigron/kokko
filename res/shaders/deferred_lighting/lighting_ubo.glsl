const int MaxLightCount = 8;
const int MaxCascadeCount = 4;

layout(std140, binding = 0) uniform Lighting
{
	int point_count;
	int spot_count;
	int shd_casc_count;

    vec2 half_near_plane;

	mat4x4 pers_mat;
	
	vec3 light_col[MaxLightCount];
	vec3 light_pos[MaxLightCount];
	vec3 light_dir[MaxLightCount];
	float light_angle[MaxLightCount];

	mat4x4 shd_mat[MaxCascadeCount];
	float shd_splits[MaxCascadeCount + 1];
    vec3 ambient_color;
} un;
