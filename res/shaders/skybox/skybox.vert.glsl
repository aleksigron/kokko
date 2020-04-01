layout(location = 0) in vec3 vert_pos;

out vec3 fs_w_direction;

void main()
{
	gl_Position = transform.MVP * vec4(vert_pos, 1.0);
	fs_w_direction = vert_pos;
}
