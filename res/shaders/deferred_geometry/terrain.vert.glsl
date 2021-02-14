layout(location = VERTEX_ATTR_INDEX_POS) in vec2 position;
layout(location = VERTEX_ATTR_INDEX_NOR) in vec3 normal;

out VS_TO_FS {
    vec3 normal;
} vs_out;

void main()
{
	vec3 N = normalize(vec3(transform.MV * vec4(0.0, 1.0, 0.0, 0.0)));

	gl_Position = transform.MVP * vec4(position.x, sin(position.x*0.25)*0.5 + sin(position.y*0.25)*0.5, position.y, 1.0);
	vs_out.normal = N;
}
