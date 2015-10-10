#version 330 core

in vec3 fs_w_norm;
in vec3 fs_vert_color;

out vec3 color;

void main()
{
	vec3 light_dir = vec3(0.0, 1.0, 0.0);
	float f = dot(fs_w_norm, light_dir);
	f = (f + 1.0) * 0.5;

	color = fs_vert_color * f;
}
