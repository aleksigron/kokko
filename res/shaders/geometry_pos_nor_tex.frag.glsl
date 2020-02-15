#version 330 core

#define PI 3.1415926538

layout(location = 0) out vec2 g_norm;
layout(location = 1) out vec4 g_alb_spec;

in vec3 fs_v_norm;
in vec2 fs_texcoord;

uniform sampler2D base_color;

void main()
{
    vec3 nor = normalize(fs_v_norm);
    g_norm = vec2(atan(nor.y, nor.x) / PI * 0.5 + 0.5, acos(nor.z) / PI);
    g_alb_spec = vec4(texture(base_color, fs_texcoord).rgb, 0.5);
}
