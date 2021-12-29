#version 450

#stage compute
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/particles/compute_common.glsl"

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main()
{
	uint particle_count = atomicOr(counters.post_sim_alive_count, 0);

    // count, primCount, first, baseInstance
	indirect_params[INDIRECT_INDEX_DRAW] = uvec4(4, particle_count, 0, 0);
}
