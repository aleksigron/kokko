#version 450
#property environment_map texCube

#stage vertex
#include "engine/shaders/common/constants.glsl"

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform SkyboxUniformBlock
{
	mat4x4 transform;
	float intensity;
}
uniforms;

layout(location = 0) in vec3 vert_pos;

out vec3 fs_w_direction;

void main()
{
	gl_Position = uniforms.transform * vec4(vert_pos, 1.0);
	fs_w_direction = vert_pos;
}

#stage fragment
#include "engine/shaders/common/constants.glsl"

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform SkyboxUniformBlock
{
	mat4x4 transform;
	float intensity;
}
uniforms;

in vec3 fs_w_direction;

out vec3 color;

uniform samplerCube environment_map;

void main()
{
	gl_FragDepth = 0.0;
	color = texture(environment_map, fs_w_direction).rgb * uniforms.intensity;
}
