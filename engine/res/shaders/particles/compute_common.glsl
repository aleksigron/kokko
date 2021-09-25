
#define GROUP_SIZE_EMIT 64
#define GROUP_SIZE_SIMULATE 256

#define INDIRECT_INDEX_EMIT 0
#define INDIRECT_INDEX_SIMULATE 1
#define INDIRECT_INDEX_DRAW 2
#define INDIRECT_COUNT 3

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform ParticleUpdateBlock
{
    vec3 emit_position;
    float emit_position_var;
    vec3 initial_velocity;
    float initial_velocity_var;
    vec3 gravity;
    int noise_texture_size;
    int noise_seed;
    int emit_count;
    float particle_lifetime;
    float particle_size;
    float bounce_energy;
    float current_time;
	float delta_time;
} uniforms;

layout(std430, binding = 0) buffer PositionBlock
{
    vec4 positions[];
};

layout(std430, binding = 1) buffer VelocityBlock
{
    vec4 velocities[];
};

layout(std430, binding = 2) buffer LifetimeBlock
{
    vec2 lifetimes[];
};

layout(std430, binding = 3) buffer DeadBlock
{
    uint dead_indices[];
};

layout(std430, binding = 4) buffer AliveCurrentBlock
{
    uint alive_indices_current[];
};

layout(std430, binding = 5) buffer AliveNextBlock
{
    uint alive_indices_next[];
};

layout(std430, binding = 6) buffer CounterBlock
{
    uint alive_count;
    uint dead_count;
    uint emit_count;
    uint post_sim_alive_count;
} counters;

layout(std430, binding = 7) buffer IndirectBlock
{
    uvec4 indirect_params[INDIRECT_COUNT];
};
