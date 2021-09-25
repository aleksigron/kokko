vec3 sample_box(sampler2D src_tex, vec2 tex_scale, vec2 uv, float delta)
{
	vec4 o = tex_scale.xyxy * vec4(-delta, -delta, delta, delta);
	vec3 s =
		texture(src_tex, uv + o.xy).rgb +
		texture(src_tex, uv + o.zy).rgb +
		texture(src_tex, uv + o.xw).rgb +
		texture(src_tex, uv + o.zw).rgb;

	return s * 0.25;
}
