#version 450

#stage vertex
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/debug/debug_normal_block.h"

layout(location = VERTEX_ATTR_INDEX_POS) in vec3 position;
layout(location = VERTEX_ATTR_INDEX_NOR) in vec3 normal;

out VS_TO_GS {
	mat4 MVP;
	vec3 position;
	vec3 normal;
} vs_out;

void main()
{
	gl_Position = MVP * vec4(position, 1.0);
	vs_out.MVP = MVP;
	vs_out.position = position;
	vs_out.normal = normal;
}

#stage geometry

#include "engine/shaders/debug/debug_normal_block.h"

in VS_TO_GS {
	mat4 MVP;
	vec3 position;
	vec3 normal;
} gs_in[1];

layout(points) in;
layout(line_strip, max_vertices = 2) out;

void main()
{
	gl_Position = gs_in[0].MVP * vec4(gs_in[0].position, 1.0);
	EmitVertex();

	gl_Position = gs_in[0].MVP * vec4(gs_in[0].position + gs_in[0].normal * normal_length, 1.0);
	EmitVertex();
}

#stage fragment
#include "engine/shaders/debug/debug_normal_block.h"

out vec4 output_color;

void main()
{
	output_color = base_color;
}
