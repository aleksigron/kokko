#version 450

#stage vertex
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/transform_block.glsl"

layout(location = VERTEX_ATTR_INDEX_POS) in vec3 position;

void main()
{
 	gl_Position = transform.MVP * vec4(position, 1.0);
}

#stage fragment

void main()
{
}
