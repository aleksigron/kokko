
out vec3 out_color;

in VS_TO_FS
{
	vec3 local_pos;
}
fs_in;

uniform sampler2D equirectangular_map;

const vec2 inv_atan = vec2(0.1591, 0.3183);
vec2 cubemap_to_spherical_coord(vec3 v)
{
    v = normalize(v);
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= inv_atan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = cubemap_to_spherical_coord(fs_in.local_pos);
    out_color = texture(equirectangular_map, uv).rgb;
}
