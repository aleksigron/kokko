in VS_TO_FS {
    vec3 normal;
} fs_in;

void main()
{
    g_albedo = vec3(1.0);
    g_normal = vec2(atan(fs_in.normal.y, fs_in.normal.x) / M_PI * 0.5 + 0.5, acos(fs_in.normal.z) / M_PI);
    g_material = vec3(metalness, roughness, 0.0);
}
