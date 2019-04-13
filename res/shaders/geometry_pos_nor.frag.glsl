#version 330 core

layout(location = 0) out vec3 g_norm;
layout(location = 1) out vec4 g_alb_spec;

in vec3 fs_v_norm;

uniform vec3 base_color;

void main()
{
    g_norm = normalize(fs_v_norm);
    g_alb_spec = vec4(base_color, 0.5);
}  