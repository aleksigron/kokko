#version 450

#stage vertex
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/transform_block.glsl"

layout(location = VERTEX_ATTR_INDEX_POS) in vec3 position;
layout(location = VERTEX_ATTR_INDEX_NOR) in vec3 normal;

out VS_TO_FS {
    vec3 N;
} vs_out;

void main()
{

	gl_Position = transform.MVP * vec4(position, 1.0);
	vs_out.N = normalize(vec3(transform.MV * vec4(normal, 0.0)));
}

#stage fragment
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/g_buffer_io.glsl"
#include "engine/shaders/common/deferred_frag_output.glsl"

in VS_TO_FS {
    vec3 N;
} fs_in;

void main()
{
    vec3 v_normal = normalize(vec4(fs_in.N, 0.0)).xyz;

    g_albedo = vec3(1.0, 0.0, 1.0);
    g_normal = pack_normal(v_normal);
    g_material = vec3(0.0, 1.0, 0.0);
}
