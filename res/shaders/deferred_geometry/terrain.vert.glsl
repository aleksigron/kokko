layout(location = VERTEX_ATTR_INDEX_POS) in vec2 position;
layout(location = VERTEX_ATTR_INDEX_NOR) in vec3 normal;

out VS_TO_FS {
    vec3 normal;
} vs_out;

uniform sampler2D height_map;

const float HeightExtent = 1.0;
const float OffsetAmount = 1.0;
const int SampleCount = 5;
const vec2 offsetVectors[SampleCount] = vec2[](
	vec2(0.0, 0.0),
	vec2(0.0, OffsetAmount),
	vec2(0.0, -OffsetAmount),
	vec2(OffsetAmount, 0.0),
	vec2(-OffsetAmount, 0.0)
);

void main()
{
	vec3 N = normalize(vec3(transform.MV * vec4(0.0, 1.0, 0.0, 0.0)));

	const float terrainSize = 64.0;

	float samples[SampleCount];

	for (int i = 0; i < SampleCount; ++i)
	{
		vec2 normalized = (position + offsetVectors[i]) / terrainSize + 0.5;
		samples[i] = texture(height_map, normalized).r;
	}

	vec3 tangent_x = vec3(OffsetAmount * 2.0, (samples[1] - samples[2]) * HeightExtent, 0.0);
	vec3 tangent_y = vec3(0.0, (samples[3] - samples[4]) * HeightExtent, OffsetAmount * 2.0);

	vec3 w_normal = cross(normalize(tangent_y), normalize(tangent_x));

	gl_Position = transform.MVP * vec4(position.x, samples[0] * HeightExtent, position.y, 1.0);
	vs_out.normal = normalize(vec3(transform.MV * vec4(w_normal, 0.0)));
}
