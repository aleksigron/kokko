
in VS_TO_FS
{
	vec2 tex_coord;
} fs_in;

out vec3 color;

uniform sampler2D source_map;

layout(std140, binding = 0) uniform BloomExtractBlock
{
	vec2 texture_scale;
	float threshold;
    float soft_threshold;
};

vec3 prefilter(vec3 c)
{
    float brightness = max(c.r, max(c.g, c.b));
    float knee = threshold * soft_threshold;
    float soft = brightness - threshold + knee;
    soft = clamp(soft, 0, 2 * knee);
    soft = soft * soft / (4 * knee + 0.00001);
    float contribution = max(soft, brightness - threshold);
    contribution /= max(brightness, 0.00001);
    return c * contribution;
}

void main()
{
    color = prefilter(sample_box(source_map, texture_scale, fs_in.tex_coord, 1.0));
}
