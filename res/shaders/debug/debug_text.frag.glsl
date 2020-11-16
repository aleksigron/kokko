in vec2 fs_tex_coord;

out vec4 color;

uniform sampler2D glyph_tex;

void main()
{
	float normal = texture(glyph_tex, fs_tex_coord).r;
	float offset = texture(glyph_tex, fs_tex_coord - vec2(0.0, shadow_offset)).r;

	float alpha = clamp(normal + offset, 0.0, 1.0);

	color = vec4(normal, normal, normal, alpha);
}
