in vec3 fs_w_direction;

out vec3 color;

uniform samplerCube environment_map;

void main()
{
	gl_FragDepth = 0.0;
	color = texture(environment_map, fs_w_direction).rgb;
}
