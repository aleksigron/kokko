in VS_TO_FS {
    vec3 normal;
	vec2 tex_coord;
} fs_in;

uniform sampler2D albedo_map;

void main()
{
    vec3 N = normalize(fs_in.normal);

    g_albedo = texture(albedo_map, fs_in.tex_coord).rgb;
    g_normal = vec2(atan(N.y, N.x) / M_PI * 0.5 + 0.5, acos(N.z) / M_PI);
    g_material = vec3(metalness, roughness, 0.0);
}
