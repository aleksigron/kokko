layout(location = 0) in vec3 position;

out VS_TO_FS
{
	vec2 tex_coord;
} vs_out;

void main()
{
	float scale = 0.05;
	vec3 particle_pos = positions[gl_InstanceID].xyz;
	vec4 v_pos = transform.MV * vec4(particle_pos, 1.0);
	gl_Position = viewport.P * (v_pos + vec4(position, 0.0) * scale);
	vs_out.tex_coord = position.xy + vec2(0.5);
}
