in vec3 fs_v_norm;
in vec2 fs_texcoord;

uniform sampler2D base_color;

void main()
{
    g_alb_spec = vec4(texture(base_color, fs_texcoord).rgb, 0.5);
    vec3 nor = normalize(fs_v_norm);
    g_norm = vec2(atan(nor.y, nor.x) / M_PI * 0.5 + 0.5, acos(nor.z) / M_PI);
    g_emissive = 0.0;
}
