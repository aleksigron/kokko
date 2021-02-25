
in VS_TO_FS
{
	vec2 tex_coord;
	vec3 eye_dir;
} fs_in;

out float color;

uniform sampler2D noise_texture;
uniform sampler2D g_normal;
uniform sampler2D g_depth;

const float bias = 0.025;

void main()
{
	float window_z = texture(g_depth, fs_in.tex_coord).r;
	vec3 surface_pos = view_pos_from_depth(window_z, perspective_mat, fs_in.eye_dir);

	vec3 normal = unpack_normal(texture(g_normal, fs_in.tex_coord).rg);

	vec3 random = vec3(texture(noise_texture, fs_in.tex_coord * noise_scale).xy * 2.0 - 1.0, 0.0);
	vec3 tangent = normalize(random - normal * dot(random, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3x3 TBN = mat3x3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < kernel_size; ++i)
	{
		vec3 sample_pos = TBN * kernel[i] * sample_radius + surface_pos;
		
		vec4 offset = perspective_mat * vec4(sample_pos, 1.0);
		offset.xyz = offset.xyz / offset.w * 0.5 + 0.5;
		
		float sample_window_z = texture(g_depth, offset.xy).r;
		float sample_depth = view_z_from_depth(sample_window_z, perspective_mat);
		float range_check = smoothstep(0.0, 1.0, sample_radius / abs(surface_pos.z - sample_depth));
		occlusion += (sample_depth >= sample_pos.z + bias ? 1.0 : 0.0) * range_check;
	}

	color = 1.0 - (occlusion / kernel_size);
}
