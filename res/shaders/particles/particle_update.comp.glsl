
layout(local_size_x = 256) in;

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform UpdateParticleBlock
{
	vec3 gravity;
	float delta_time;
};

void main()
{
    uint particle_idx = gl_GlobalInvocationID.x;

    vec3 pos = positions[particle_idx].xyz;
    vec3 vel = velocities[particle_idx].xyz;

    if (pos.y + vel.y * delta_time < 0.0)
        vel.y = abs(vel.y);
    
    pos += vel * delta_time * 0.5;
    vel += gravity * delta_time;
    pos += vel * delta_time * 0.5;

    positions[particle_idx] = vec4(pos, 0.0);
    velocities[particle_idx] = vec4(vel, 0.0);
}
