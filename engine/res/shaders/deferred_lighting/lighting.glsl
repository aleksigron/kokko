#version 450

#property g_albedo tex2d
#property g_normal tex2d
#property g_material tex2d
#property g_depth tex2d
#property ssao_map tex2d
#property shadow_map tex2d
#property diff_irradiance_map texCube
#property spec_irradiance_map texCube
#property brdf_lut tex2d

#stage vertex

#include "res/shaders/common/constants.glsl"
#include "res/shaders/deferred_lighting/lighting_ubo.glsl"

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

#include "res/shaders/common/constants.glsl"
#include "res/shaders/common/g_buffer_io.glsl"
#include "res/shaders/deferred_lighting/lighting_ubo.glsl"

in VS_TO_FS
{
	vec2 tex_coord;
	vec3 eye_dir;
} fs_in;

out vec3 color;

uniform sampler2D g_albedo;
uniform sampler2D g_normal;
uniform sampler2D g_material;
uniform sampler2D g_depth;
uniform sampler2D ssao_map;
uniform sampler2DShadow shadow_map;
uniform samplerCube diff_irradiance_map;
uniform samplerCube spec_irradiance_map;
uniform sampler2D brdf_lut;

vec3 fresnel_schlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnel_schlick_roughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float distribution_ggx(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	
	float num = a2;
	float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
	denom = M_PI * denom * denom;
	
	return num / denom;
}

float geo_schlick_ggx(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}

float geometry_smith(float NdotV, float NdotL, float roughness)
{
	float ggx2  = geo_schlick_ggx(NdotV, roughness);
	float ggx1  = geo_schlick_ggx(NdotL, roughness);
	
	return ggx1 * ggx2;
}

vec3 calc_light(vec3 F0, vec3 N, vec3 V, vec3 L, vec3 albedo, vec3 light_col, float metalness, float roughness)
{
	vec3 H = normalize(V + L);
	float NdotV = abs(dot(N, V)) + 1e-5; // avoid artifact
	float NdotH = clamp(dot(N, H), 0.0, 1.0);
	float NdotL = clamp(dot(N, L), 0.0, 1.0);
	float HdotV = clamp(dot(H, V), 0.0, 1.0);

	float NDF = distribution_ggx(NdotH, roughness);
	float G = geometry_smith(NdotV, NdotL, roughness);
	vec3 F = fresnel_schlick(HdotV, F0);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * NdotV * NdotL;
	vec3 specular = numerator / max(denominator, 0.001);

	vec3 kD = (vec3(1.0) - F) * (1.0 - metalness);

	return (kD * albedo / M_PI + specular) * light_col * NdotL;
}

float smooth_distance_att(float sqr_distance, float inv_sqr_att_radius)
{
	float factor = sqr_distance * inv_sqr_att_radius;
	float smooth_factor = clamp(1.0f - factor * factor, 0.0, 1.0);
	return smooth_factor * smooth_factor;
}

float get_distance_att(vec3 light_vector, float inv_sqr_att_radius)
{
	float sqr_dist = dot(light_vector, light_vector);
	float attenuation = 1.0 / max(sqr_dist, 0.01*0.01);
	attenuation *= smooth_distance_att(sqr_dist, inv_sqr_att_radius);
	return attenuation;
}

float offset_lookup(sampler2DShadow map, vec4 loc, vec2 offset)
{
	return textureProj(map, vec4(loc.xy + offset * shadow_map_scale * loc.w, loc.z, loc.w));
}

void main()
{
	vec3 albedo = texture(g_albedo, fs_in.tex_coord).rgb;
	vec3 N = unpack_normal(texture(g_normal, fs_in.tex_coord).rg);

	vec3 material = texture(g_material, fs_in.tex_coord).rgb;
	float metalness = material.r;
	float roughness = material.g;

	float window_z = texture(g_depth, fs_in.tex_coord).r;
	vec3 surface_pos = view_pos_from_depth(window_z, perspective_mat, fs_in.eye_dir);
	vec3 V = normalize(-surface_pos);
	vec3 F0 = mix(vec3(0.04), albedo, metalness);
	vec3 Lo = vec3(0.0);
	
	int light_idx = 0;

	for (int end = dir_count; light_idx < end; ++light_idx)
	{
		float view_z = surface_pos.z;

		// Select correct shadow cascade
		int cascade_index = 0;
		for (int i = 0; i < shadow_casc_count; ++i)
			if (shadow_splits[i] <= -view_z && shadow_splits[i + 1] > -view_z)
				cascade_index = i;

		vec3 L = -light_dir[light_idx].xyz;
		
		float shadow_coeff = 1.0;

		if (light_shadow[light_idx] == true)
		{
			// Get shadow depth
			vec4 shadow_coord = shadow_mats[cascade_index] * vec4(surface_pos, 1.0);
			shadow_coord.x = (cascade_index + shadow_coord.x) / shadow_casc_count;
			float NdotL = max(dot(N, L), 0.0);
			shadow_coord.z += shadow_bias_offset + clamp(shadow_bias_factor * tan(acos(NdotL)), 0.0, shadow_bias_clamp);

			shadow_coeff = 0.0;
			for (float y = -1.5; y <= 1.5; y += 1.0)
				for (float x = -1.5; x <= 1.5; x += 1.0)
					shadow_coeff += offset_lookup(shadow_map, shadow_coord, vec2(x, y)); 
					
			shadow_coeff /= 16.0;
		}
		
		Lo += calc_light(F0, N, V, L, albedo, light_col[light_idx], metalness, roughness) * shadow_coeff;
	}

	for (int end = dir_count + point_count; light_idx < end; ++light_idx)
	{
		vec3 surface_to_light = light_pos[light_idx].xyz - surface_pos;
		float attenuation = get_distance_att(surface_to_light, light_pos[light_idx].w);

		vec3 L = normalize(surface_to_light);
		Lo += calc_light(F0, N, V, L, albedo, light_col[light_idx], metalness, roughness) * attenuation;
	}
	
	for (int end = dir_count + point_count + spot_count; light_idx < end; ++light_idx)
	{
		vec3 surface_to_light = light_pos[light_idx].xyz - surface_pos;
		vec3 L = normalize(surface_to_light);

		float direction_asin = asin(dot(L, light_dir[light_idx].xyz));
		float direction_att = clamp((direction_asin - (M_HPI) + light_dir[light_idx].w) * 20, 0.0, 1.0);

		float distance_att = get_distance_att(surface_to_light, light_pos[light_idx].w);
		
		Lo += calc_light(F0, N, V, L, albedo, light_col[light_idx], metalness, roughness) * direction_att * distance_att;
	}

    vec3 F = fresnel_schlick_roughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = (1.0 - F) * (1.0 - metalness);
	
	vec3 v_reflect = reflect(-V, N);
	vec3 w_reflect = (view_to_world * vec4(v_reflect, 0.0)).xyz;

    const float MAX_REFLECTION_LOD = 5.0;
    vec3 spec_color = textureLod(spec_irradiance_map, w_reflect, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 env_brdf = texture(brdf_lut, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 spec_irr = spec_color * (F * env_brdf.x + env_brdf.y);

	vec3 w_normal = (view_to_world * vec4(N, 0.0)).xyz;
	vec3 diff_irr = texture(diff_irradiance_map, w_normal).rgb * kD;

	float ao = texture(ssao_map, fs_in.tex_coord).r;

	vec3 ambient = (diff_irr + spec_irr) * albedo * ao;

	color = ambient + Lo;
}
