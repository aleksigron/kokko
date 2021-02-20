layout(location = VERTEX_ATTR_INDEX_POS) in vec2 position;
layout(location = VERTEX_ATTR_INDEX_NOR) in vec3 normal;

out VS_TO_FS {
    vec3 normal;
	vec2 tex_coord;
} vs_out;

uniform sampler2D height_map;

float sample_height(vec2 offset)
{
	vec2 normalized = (position + offset) / uniforms.terrain_size + 0.5;
	return texture(height_map, normalized).r;
}

void main()
{
	const float offset_amount = 1.0;
	float y_extent = uniforms.max_height - uniforms.min_height;

	float h0 = sample_height(vec2(0.0, 0.0));
	float h1 = sample_height(vec2(0.0, offset_amount));
	float h2 = sample_height(vec2(0.0, -offset_amount));
	float h3 = sample_height(vec2(offset_amount, 0.0));
	float h4 = sample_height(vec2(-offset_amount, 0.0));

	vec3 tangent_x = vec3(offset_amount * 2.0, (h1 - h2) * y_extent, 0.0);
	vec3 tangent_y = vec3(0.0, (h3 - h4) * y_extent, offset_amount * 2.0);
	vec3 w_normal = cross(normalize(tangent_y), normalize(tangent_x));

	gl_Position = uniforms.MVP * vec4(position.x, uniforms.min_height + h0 * y_extent, position.y, 1.0);
	vs_out.normal = normalize(vec3(uniforms.MV * vec4(w_normal, 0.0)));
	vs_out.tex_coord = position * uniforms.texture_scale;
}
