uniform mat4 mat_v;

float sample_ao(vec2 texcoord, vec3 position, vec3 normal, float range, float bias_value)
{
    vec3 ao_direction = texture2D(sampler_02, texcoord).xyz - position;
    float ao_distance = length(ao_direction);

    if (ao_distance <= 0.00001)
    {
        return 0.0;
    }

    float intensity = clamp(
        (dot(ao_direction / ao_distance, normal) - bias_value) /
        (1.0 + ao_distance),
        0.0,
        1.0);
    float attenuation = 1.0 - smoothstep(range * 0.5, range, ao_distance);
    return intensity * attenuation;
}

void main()
{
    const float range = 1.2;
    const float bias_value = 0.6;
    const float sin_45 = 0.707;
    const int iterations = 4;
    const vec2 directions[4] = vec2[](
        vec2(1.0, 0.0),
        vec2(-1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(0.0, -1.0));

    vec4 normal_sample = texture2D(sampler_01, v_texcoord);
    vec3 position = texture2D(sampler_02, v_texcoord).xyz;
    vec4 position_in_view_space = mat_v * vec4(position, 1.0);
    vec2 random_direction = texture2D(sampler_03, v_texcoord * 32.0).xz;

    float ao = 0.0;
    if (normal_sample.w > 0.0 && abs(position_in_view_space.z) > 0.00001)
    {
        float radius = range / position_in_view_space.z;

        for (int i = 0; i < iterations; ++i)
        {
            vec2 texcoord_offset_1 = reflect(directions[i], random_direction);
            vec2 texcoord_offset_2 = vec2(
                texcoord_offset_1.x * sin_45 - texcoord_offset_1.y * sin_45,
                texcoord_offset_1.x * sin_45 + texcoord_offset_1.y * sin_45);

            texcoord_offset_1 *= radius;
            texcoord_offset_2 *= radius;

            ao += sample_ao(v_texcoord + texcoord_offset_1 * 0.25, position, normal_sample.xyz, range, bias_value);
            ao += sample_ao(v_texcoord + texcoord_offset_2 * 0.50, position, normal_sample.xyz, range, bias_value);
            ao += sample_ao(v_texcoord + texcoord_offset_1 * 0.75, position, normal_sample.xyz, range, bias_value);
            ao += sample_ao(v_texcoord + texcoord_offset_2, position, normal_sample.xyz, range, bias_value);
        }
    }

    ao /= float(iterations) * 4.0;
    gl_FragColor = vec4(vec3(pow(1.0 - ao, 24.0)), 1.0);
}
