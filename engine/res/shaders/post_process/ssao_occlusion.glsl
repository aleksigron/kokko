#version 450
#property noise_texture tex2d
#property g_normal tex2d
#property g_depth tex2d

#stage vertex
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/post_process/ssao_occlusion_uniform.glsl"

layout(location = 0) in vec3 ndc_pos;

out VS_TO_FS
{
	vec2 tex_coord;
	vec3 eye_dir;
}
vs_out;

void main()
{
	vs_out.tex_coord = ndc_pos.xy * 0.5 + vec2(0.5, 0.5);
	vs_out.eye_dir = vec3((2.0 * half_near_plane * vs_out.tex_coord) - half_near_plane, -1.0);
	gl_Position = vec4(ndc_pos, 1.0);
}

#stage fragment
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/g_buffer_io.glsl"
#include "engine/shaders/post_process/ssao_occlusion_uniform.glsl"

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
		vec3 sample_pos_v = TBN * kernel[i] * sample_radius + surface_pos;
		vec4 sample_pos_c = perspective_mat * vec4(sample_pos_v, 1.0);
		vec2 sample_uv = sample_pos_c.xy / sample_pos_c.w * 0.5 + 0.5;
		
		float sample_window_z = texture(g_depth, sample_uv).r;
		float sample_z = -view_z_from_depth(sample_window_z, perspective_mat);
		float range_check = smoothstep(0.0, 1.0, sample_radius / abs(surface_pos.z - sample_z));
		occlusion += (sample_z >= sample_pos_v.z + bias ? 1.0 : 0.0) * range_check;
	}

	color = 1.0 - (occlusion / kernel_size);
}
