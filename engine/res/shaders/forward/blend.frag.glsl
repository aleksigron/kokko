
in vec3 fs_world_norm;

out vec4 color;

void main()
{
	vec3 w_nor = normalize(fs_world_norm);
	vec3 col = w_nor * 0.5 + vec3(0.5);

	vec3 v_nor = (viewport.V * vec4(w_nor, 0.0)).rgb;
	float a = 1.0 - abs(dot(v_nor, vec3(0.0, 0.0, 1.0)));

	color = vec4(col, a);
}
