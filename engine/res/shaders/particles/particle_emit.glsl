#version 450
#property noise_texture tex2d

#stage compute
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/particles/compute_common.glsl"

layout(local_size_x = GROUP_SIZE_EMIT) in;

uniform sampler2D noise_texture;

vec4 fetch_noise(uint index)
{
    int noise_x = (uniforms.noise_seed + int(index)) % uniforms.noise_texture_size;
    int noise_y = (uniforms.noise_seed + int(index)) / uniforms.noise_texture_size;
    return texelFetch(noise_texture, ivec2(noise_x, noise_y), 0);
}

void main()
{
	if (gl_GlobalInvocationID.x < counters.emit_count)
	{
        // new particle index retrieved from dead list (pop):
        uint dead_count = atomicAdd(counters.dead_count, -1);
        uint particle_idx = dead_indices[dead_count - 1];

        vec4 pos_noise = fetch_noise(gl_GlobalInvocationID.x * 2 + 0);
        vec4 vel_noise = fetch_noise(gl_GlobalInvocationID.x * 2 + 1);

        vec3 pos = uniforms.emit_position + (pos_noise.xyz - vec3(0.5)) * 2.0 * uniforms.emit_position_var;
        vec3 vel = uniforms.initial_velocity + (vel_noise.xyz - vec3(0.5)) * 2.0 * uniforms.initial_velocity_var;

        // write out the new particle:
        positions[particle_idx] = vec4(pos, uniforms.particle_size);
        velocities[particle_idx] = vec4(vel, 0.0);
        lifetimes[particle_idx] = vec2(0.0, uniforms.particle_lifetime);

        // and add index to the alive list (push):
        uint alive_count = atomicAdd(counters.alive_count, 1);
        alive_indices_current[alive_count] = particle_idx;
    }
}
