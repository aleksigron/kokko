
const int MaxKernelSize = 64;

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform SsaoOcclusionBlock
{
	vec3 kernel[MaxKernelSize];
	mat4x4 perspective_mat;
	vec2 half_near_plane;
	vec2 noise_scale;
	float sample_radius;
    int kernel_size;
};
