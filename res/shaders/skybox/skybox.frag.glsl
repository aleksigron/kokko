in vec3 fs_w_direction;

out vec3 color;

uniform samplerCube tex;

void main()
{
	gl_FragDepth = 0.0;
	color = texture(tex, fs_w_direction).rgb;
}
