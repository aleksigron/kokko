#version 330 core

#define PI 3.1415926538

layout(location = 0) out vec4 g_alb_spec;
layout(location = 1) out vec2 g_norm;
layout(location = 2) out float g_emissive;

in vec3 fs_v_norm;

uniform vec3 base_color;

void main()
{
    g_alb_spec = vec4(base_color, 0.5);
    vec3 nor = normalize(fs_v_norm);
    g_norm = vec2(atan(nor.y, nor.x) / PI * 0.5 + 0.5, acos(nor.z) / PI);
    g_emissive = 0.0;
}
