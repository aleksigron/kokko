#version 330 core

in vec2 fs_tex_coord;

out vec3 color;

uniform sampler2D glyph_texture;

void main()
{
	color = texture(glyph_texture, fs_tex_coord).rrr;
}
