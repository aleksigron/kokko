#include "engine/shaders/common/constants.glsl"

layout(std140, binding = BLOCK_BINDING_OBJECT) uniform DebugNormalBlock
{
	mat4x4 MVP;
	mat4x4 MV;
	vec4 base_color;
	float normal_length;
};
