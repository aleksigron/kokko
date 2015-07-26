#version 330 core

in vec2 fragTexCoord;

out vec3 color;

uniform sampler2D tex;

void main()
{
	color = texture(tex, fragTexCoord).rgb;
}