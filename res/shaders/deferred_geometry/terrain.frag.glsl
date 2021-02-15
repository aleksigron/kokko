in VS_TO_FS {
    vec3 normal;
} fs_in;

void main()
{
    vec3 N = normalize(fs_in.normal);

    g_albedo = vec3(1.0);
    g_normal = vec2(atan(N.y, N.x) / M_PI * 0.5 + 0.5, acos(N.z) / M_PI);
    g_material = vec3(metalness, roughness, 0.0);
}
