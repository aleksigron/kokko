#version 450

#stage vertex
#include "res/shaders/common/constants.glsl"
#include "res/shaders/common/transform_block.glsl"

layout(location = 0) in vec3 position;

void main()
{
 	gl_Position = transform.MVP * vec4(position, 1.0);
}

#stage fragment
layout(location = 0) out float depth;

void main()
{
    depth = gl_FragCoord.z;
}
