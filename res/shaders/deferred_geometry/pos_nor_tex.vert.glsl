layout(location = VERTEX_ATTR_INDEX_POS) in vec3 position;
layout(location = VERTEX_ATTR_INDEX_NOR) in vec3 normal;
layout(location = VERTEX_ATTR_INDEX_UV0) in vec2 texcoord;

out vec3 fs_v_norm;
out vec2 fs_texcoord;

void main()
{
	gl_Position = transform.MVP * vec4(position, 1.0);
	fs_v_norm = (transform.MV * vec4(normal, 0.0)).xyz;
	fs_texcoord = texcoord;
}
