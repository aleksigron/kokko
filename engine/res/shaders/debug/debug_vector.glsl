#version 450

#stage vertex
#include "engine/shaders/debug/debug_vector_uniform.glsl"

layout(location = 0) in vec3 position;

void main()
{
	gl_Position = transform * vec4(position, 1.0);
}

#stage fragment
#include "engine/shaders/debug/debug_vector_uniform.glsl"

out vec4 output_color;

void main()
{
	output_color = base_color;
}
