#version 450

#stage vertex
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/transform_block.glsl"
#include "engine/shaders/common/viewport_block.glsl"

layout(location = 0) in vec3 vertex_pos;

out VS_TO_FS
{
	vec2 tex_coord;
	float age;
} vs_out;

layout(std430, binding = 0) readonly buffer PositionBlock
{
    vec4 positions[];
};

layout(std430, binding = 2) readonly buffer LifetimeBlock
{
    vec2 lifetimes[];
};

layout(std430, binding = 5) buffer AliveBlock
{
    uint alive_indices[];
};

void main()
{
	vec4 pos4 = positions[alive_indices[gl_InstanceID]];
	vec2 life = lifetimes[alive_indices[gl_InstanceID]];
	float scale = pos4.w;
	vec3 particle_pos = pos4.xyz;
	vec4 v_pos = transform.MV * vec4(particle_pos, 1.0);
	gl_Position = viewport.P * (v_pos + vec4(vertex_pos, 0.0) * scale);
	vs_out.tex_coord = vertex_pos.xy + vec2(0.5);
	vs_out.age = clamp(life.x / life.y, 0.0, 1.0);
}

#stage fragment

in VS_TO_FS
{
	vec2 tex_coord;
	float age;
} fs_in;

out vec4 color;

void main()
{
	float age_alpha = 1.0 - (fs_in.age * fs_in.age * fs_in.age);
	float pos_alpha = smoothstep(0.5, 0.0, length(fs_in.tex_coord - vec2(0.5)));
	color = vec4(1.5, 1.5, 1.5, pos_alpha * age_alpha);
}
