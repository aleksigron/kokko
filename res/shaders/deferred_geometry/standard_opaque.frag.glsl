in VS_TO_FS {
    vec2 tex_coord;
    mat3 TBN;
} fs_in;

uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D roughness_map;

void main()
{
    vec3 tan_normal = texture(normal_map, fs_in.tex_coord).rgb;
    vec3 v_normal = normalize(fs_in.TBN * (tan_normal * 2.0 - 1.0));
    float tex_roughness = texture(roughness_map, fs_in.tex_coord).r;

    g_albedo = texture(albedo_map, fs_in.tex_coord).rgb;
    g_normal = vec2(atan(v_normal.y, v_normal.x) / M_PI * 0.5 + 0.5, acos(v_normal.z) / M_PI);
    g_material = vec3(metalness, tex_roughness * roughness, 0.0);
}
