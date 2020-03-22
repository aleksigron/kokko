#define PI 3.1415926538

#define DIR_LIGHT_INDEX 0

#define SPEC_POWER 40
#define ATT_CONST_FAC 1
#define ATT_LIN_FAC 0.5

#define MAX_LIGHT_COUNT 8
#define MAX_CASCADE_COUNT 4

layout(std140, binding = 0) uniform Lighting
{
	layout(offset = 0) int point_count;
	layout(offset = 4) int spot_count;
	layout(offset = 8) int shd_casc_count;

    layout(offset = 16) vec2 half_near_plane;

	layout(offset = 32) mat4x4 pers_mat;
	
	layout(offset = 96) vec3 light_col[MAX_LIGHT_COUNT];
	layout(offset = 224) vec3 light_pos[MAX_LIGHT_COUNT];
	layout(offset = 352) vec3 light_dir[MAX_LIGHT_COUNT];
	layout(offset = 480) float light_angle[MAX_LIGHT_COUNT];

	layout(offset = 608) mat4x4 shd_mat[MAX_CASCADE_COUNT];
	layout(offset = 864) float shd_splits[MAX_CASCADE_COUNT + 1];
} un;
