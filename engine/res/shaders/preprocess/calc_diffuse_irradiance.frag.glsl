
out vec3 out_color;

in VS_TO_FS
{
	vec3 local_pos;
}
fs_in;

uniform samplerCube environment_map;

void main()
{
    // the sample direction equals the hemisphere's orientation 
    vec3 normal = normalize(fs_in.local_pos);
  
    vec3 irradiance = vec3(0.0);

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, normal);
    up         = cross(normal, right);

    float sampleDelta = 0.025;
    float nrSamples = 0.0; 
    for (float phi = 0.0; phi < 2.0 * M_PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * M_PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangent_vec = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sample_vec = tangent_vec.x * right + tangent_vec.y * up + tangent_vec.z * normal;

            irradiance += texture(environment_map, sample_vec).rgb * cos(theta) * sin(theta);
            nrSamples += 1.0;
        }
    }

    out_color = M_PI * irradiance / nrSamples;
}
