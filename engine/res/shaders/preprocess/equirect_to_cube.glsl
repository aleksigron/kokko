#version 450
#property equirectangular_map tex2d

#stage vertex
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/viewport_block.glsl"

layout (location = VERTEX_ATTR_INDEX_POS) in vec3 position;

out VS_TO_FS
{
	vec3 local_pos;
}
vs_out;

void main()
{
    gl_Position = viewport.VP * vec4(position, 1.0);
    vs_out.local_pos = position;
}

#stage fragment

out vec3 out_color;

in VS_TO_FS
{
	vec3 local_pos;
}
fs_in;

uniform sampler2D equirectangular_map;

const vec2 inv_atan = vec2(0.1591, 0.3183);
vec2 cubemap_to_spherical_coord(vec3 v)
{
    v = normalize(v);
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= inv_atan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = cubemap_to_spherical_coord(fs_in.local_pos);
    out_color = texture(equirectangular_map, uv).rgb;
}
