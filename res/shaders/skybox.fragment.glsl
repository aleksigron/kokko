#version 330 core

in vec3 fs_w_direction;

out vec3 color;

uniform samplerCube tex;

void main()
{
	color = texture(tex, fs_w_direction).rgb;
}
