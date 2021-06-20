in VS_TO_FS
{
	vec2 tex_coord;
}
fs_in;

out vec4 color;

uniform sampler2D glyph_tex;

void main()
{
	float val = texture(glyph_tex, fs_in.tex_coord).r;
	float alpha = 0.4 + val * 0.6;

	color = vec4(val, val, val, alpha);
}
