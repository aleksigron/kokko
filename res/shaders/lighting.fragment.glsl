#version 330 core

in vec2 tex_coord;

out vec3 color;

uniform sampler2D g_pos;
uniform sampler2D g_norm;
uniform sampler2D g_alb_spec;

uniform vec3 invLightDir;
uniform vec3 lightCol;

void main()
{             
    vec3 pos = texture(g_pos, tex_coord).rgb;
    vec3 norm = texture(g_norm, tex_coord).rgb;
	vec4 albSpec = texture(g_alb_spec, tex_coord);

    vec3 alb = albSpec.rgb;
	vec3 diffuse = max(dot(norm, invLightDir), 0.0) * alb * lightCol;
    
    color = diffuse;
}  