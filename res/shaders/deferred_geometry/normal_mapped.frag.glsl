in VS_TO_FS {
    vec2 tex_coord;
    mat3 TBN;
} fs_in;

uniform sampler2D base_color;
uniform sampler2D normal_map;

void main()
{
    vec3 tan_normal = texture(normal_map, fs_in.tex_coord).rgb;
    vec3 v_normal = normalize(fs_in.TBN * (tan_normal * 2.0 - 1.0));

    g_alb_spec = vec4(texture(base_color, fs_in.tex_coord).rgb, spec_intensity);
    g_norm = vec2(atan(v_normal.y, v_normal.x) / M_PI * 0.5 + 0.5, acos(v_normal.z) / M_PI);
    g_emissive = 0.0;
}
