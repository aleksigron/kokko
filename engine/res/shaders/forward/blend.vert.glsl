layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 fs_world_norm;

void main()
{
	gl_Position = transform.MVP * vec4(position, 1.0);
	fs_world_norm = (transform.M * vec4(normal, 0.0)).xyz;
}
