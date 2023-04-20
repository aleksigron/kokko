#include "engine/shaders/common/constants.glsl"

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform DebugVectorBlock
{
	mat4x4 transform;
	vec4 base_color;
};
