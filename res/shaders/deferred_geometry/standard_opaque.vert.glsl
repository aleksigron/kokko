layout(location = VERTEX_ATTR_INDEX_POS) in vec3 position;
layout(location = VERTEX_ATTR_INDEX_NOR) in vec3 normal;
layout(location = VERTEX_ATTR_INDEX_TAN) in vec3 tangent;
layout(location = VERTEX_ATTR_INDEX_UV0) in vec2 tex_coord;

out VS_TO_FS {
    vec2 tex_coord;
    mat3 TBN;
} vs_out;

void main()
{
	vec3 N = normalize(vec3(transform.MV * vec4(normal, 0.0)));
	vec3 T = normalize(vec3(transform.MV * vec4(tangent, 0.0)));
	vec3 B = cross(N, T);

	gl_Position = transform.MVP * vec4(position, 1.0);
	vs_out.tex_coord = tex_coord;
	vs_out.TBN = mat3(T, B, N);
}
