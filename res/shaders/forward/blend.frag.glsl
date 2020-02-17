#version 330 core

in vec3 fs_world_norm;

out vec4 color;

uniform mat4x4 _V;

void main()
{
	vec3 w_nor = normalize(fs_world_norm);
	vec3 col = w_nor * 0.5 + vec3(0.5);

	vec3 v_nor = (_V * vec4(w_nor, 0.0)).rgb;
	float a = 1.0 - abs(dot(v_nor, vec3(0.0, 0.0, 1.0)));

	color = vec4(col, a);
}
