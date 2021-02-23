vec3 unpack_normal(vec2 packed_normal)
{
	float a = (packed_normal.r - 0.5) * M_TAU;
	float d = cos((packed_normal.g - 0.5) * M_PI);
	return vec3(cos(a) * d, sin(a) * d, cos(packed_normal.g * M_PI));
}

vec2 pack_normal(vec3 normal)
{
    return vec2(atan(normal.y, normal.x) / M_PI * 0.5 + 0.5, acos(normal.z) / M_PI);
}

float view_z_from_depth(float window_z, mat4x4 projection)
{
	float ndc_z = 2.0 * window_z - 1.0;
	return projection[3][2] / ((projection[2][3] * ndc_z) - projection[2][2]);
}

vec3 view_pos_from_depth(float window_z, mat4x4 projection, vec3 eye_dir)
{
	float ndc_z = 2.0 * window_z - 1.0;
	float view_z = projection[3][2] / ((projection[2][3] * ndc_z) - projection[2][2]);
	return eye_dir * -view_z;
}
