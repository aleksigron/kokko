#version 330 core

in vec2 fs_tex_coord;

out vec4 color;

uniform sampler2D glyph_texture;

void main()
{
	float alpha = texture(glyph_texture, fs_tex_coord).r;
	color = vec4(1.0, 1.0, 1.0, alpha);
}
