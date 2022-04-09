#version 450
#property albedo_map tex2d
#property normal_map tex2d
#property roughness_map tex2d
#property color_tint vec3
#property metalness float
#property roughness float

#stage vertex
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/transform_block.glsl"

layout(location = VERTEX_ATTR_INDEX_POS) in vec3 position;
layout(location = VERTEX_ATTR_INDEX_NOR) in vec3 normal;
layout(location = VERTEX_ATTR_INDEX_UV0) in vec2 tex_coord;

out VS_TO_FS {
	vec3 surface_to_view;
	vec3 normal;
	vec2 tex_coord;
} vs_out;

void main()
{
	gl_Position = transform.MVP * vec4(position, 1.0);
	vs_out.surface_to_view = -(transform.MV * vec4(position, 1.0)).xyz;
	vs_out.normal = (transform.MV * vec4(normal, 0.0)).xyz;
	vs_out.tex_coord = tex_coord;
}

#stage fragment
#include "engine/shaders/common/constants.glsl"
#include "engine/shaders/common/g_buffer_io.glsl"
#include "engine/shaders/common/deferred_frag_output.glsl"

in VS_TO_FS {
	vec3 surface_to_view;
	vec3 normal;
	vec2 tex_coord;
} fs_in;

uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D roughness_map;

// Normal mapping without precomputed tangents:
// http://www.thetenthplanet.de/archives/1180

#define WITH_NORMALMAP
#define WITH_NORMALMAP_UNSIGNED

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
	// get edge vec­tors of the pix­el tri­an­gle
	vec3 dp1 = dFdx(p);
	vec3 dp2 = dFdy(p);
	vec2 duv1 = dFdx(uv);
	vec2 duv2 = dFdy(uv);

	// solve the lin­ear sys­tem
	vec3 dp2perp = cross(dp2, N);
	vec3 dp1perp = cross(N, dp1);
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// con­struct a scale-invari­ant frame
	float invmax = inversesqrt(max(dot(T,T), dot(B,B)));
	return mat3(T * invmax, B * invmax, N);
}

vec3 perturb_normal(vec3 N, vec3 V, vec2 tex_coord)
{
	// assume N, the inter­po­lat­ed ver­tex nor­mal and 
	// V, the view vec­tor (ver­tex to eye)
	vec3 map = texture2D(normal_map, tex_coord).xyz;
#ifdef WITH_NORMALMAP_UNSIGNED
	map = map * 255./127. - 128./127.;
#endif
#ifdef WITH_NORMALMAP_2CHANNEL
	map.z = sqrt(1. - dot(map.xy, map.xy));
#endif
#ifdef WITH_NORMALMAP_GREEN_UP
	map.y = -map.y;
#endif
	mat3 TBN = cotangent_frame(N, -V, tex_coord);
	return normalize(TBN * map);
}


void main()
{
	vec3 N = normalize(fs_in.normal);

#ifdef WITH_NORMALMAP
	N = perturb_normal(N, fs_in.surface_to_view, fs_in.tex_coord);
#endif

	float tex_roughness = texture(roughness_map, fs_in.tex_coord).r;

	g_albedo = texture(albedo_map, fs_in.tex_coord).rgb * color_tint;
	g_normal = pack_normal(N);
	g_material = vec3(metalness, tex_roughness * roughness, 0.0);
}
