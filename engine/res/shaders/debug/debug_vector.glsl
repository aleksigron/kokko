#version 450
#property base_color vec4

#stage vertex
#include "res/shaders/common/constants.glsl"
#include "res/shaders/common/transform_block.glsl"

layout(location = 0) in vec3 position;

void main()
{
	gl_Position = transform.MVP * vec4(position, 1.0);
}

#stage fragment
out vec4 output_color;

void main()
{
	output_color = base_color;
}
