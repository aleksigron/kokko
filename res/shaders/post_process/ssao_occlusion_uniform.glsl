
const int KernelSize = 64;

layout(std140, binding = 0) uniform SsaoOcclusionBlock
{
	vec3 kernel[KernelSize];
	mat4x4 perspective_mat;
	vec2 half_near_plane;
	vec2 noise_scale;
	float sample_radius;
};
