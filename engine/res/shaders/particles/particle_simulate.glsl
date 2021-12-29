#version 450

#stage compute
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/particles/compute_common.glsl"

layout(local_size_x = GROUP_SIZE_SIMULATE) in;

void main()
{
    uint alive_count = atomicOr(counters.alive_count, 0);

    if (gl_GlobalInvocationID.x < alive_count)
	{
		uint particle_idx = alive_indices_current[gl_GlobalInvocationID.x];
        
        vec2 life = lifetimes[particle_idx];

		if (life.x < life.y)
		{
            vec4 pos4 = positions[particle_idx];
            vec3 pos = pos4.xyz;
            vec3 vel = velocities[particle_idx].xyz;

            if (pos.y + vel.y * uniforms.delta_time < 0.0)
                vel.y = vel.y * -uniforms.bounce_energy;
            
            pos += vel * uniforms.delta_time * 0.5;
            vel += uniforms.gravity * uniforms.delta_time;
            pos += vel * uniforms.delta_time * 0.5;

            life.x += uniforms.delta_time;

            positions[particle_idx] = vec4(pos, pos4.w);
            velocities[particle_idx] = vec4(vel, 0.0);
            lifetimes[particle_idx] = life;

            uint new_alive_index = atomicAdd(counters.post_sim_alive_count, 1);
			alive_indices_next[new_alive_index] = particle_idx;
        }
		else
		{
			uint dead_index = atomicAdd(counters.dead_count, 1);
			dead_indices[dead_index] = particle_idx;
		}
    }
}
