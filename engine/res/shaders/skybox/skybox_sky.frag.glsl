in vec3 fs_w_direction;

out vec3 color;

void main()
{
	gl_FragDepth = 0.0;
	vec3 dir = normalize(fs_w_direction);
	if (dir.y < 0.0)
		color = mix(color_below_horizon, color_horizon, dir.y + 1.0);
	else
		color = mix(color_horizon, color_zenith, dir.y);
}
