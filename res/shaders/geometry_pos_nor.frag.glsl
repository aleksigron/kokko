#version 330 core

layout(location = 0) out vec3 g_pos;
layout(location = 1) out vec3 g_norm;
layout(location = 2) out vec4 g_alb_spec;

in vec3 fs_w_pos;
in vec3 fs_w_norm;

uniform vec3 base_color;

void main()
{    
    g_pos = fs_w_pos;
    g_norm = normalize(fs_w_norm);
    g_alb_spec = vec4(base_color, 0.5);
}  