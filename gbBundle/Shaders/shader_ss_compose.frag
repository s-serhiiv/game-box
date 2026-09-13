uniform vec4 motion_direction;
uniform vec4 vignetting_color;
uniform vec4 parameters_01;

vec3 adjust_saturation(vec3 color, float saturation)
{
    float grey = dot(color, vec3(0.3, 0.59, 0.11));
    return mix(vec3(grey), color, saturation);
}

void main()
{
    const float bloom_intensity = 2.0;
    const float original_intensity = 2.0;
    const float bloom_saturation = 0.75;
    const float original_saturation = 1.0;

    vec2 direction = motion_direction.xy * 3.0;
    float motion_blur_power = motion_direction.z;
    vec4 color = texture2D(sampler_01, v_texcoord);

    if (motion_blur_power > 0.0)
    {
        vec4 motion_color = vec4(0.0, 0.0, 0.0, 1.0);
        vec2 texcoord = v_texcoord;
        for (int i = 0; i < 4; ++i)
        {
            texcoord -= 0.001 * direction * motion_blur_power;
            vec4 mask_color = texture2D(sampler_04, texcoord);
            motion_color += mix(texture2D(sampler_01, texcoord), color, 1.0 - mask_color.r);
        }
        motion_color /= 4.0;
        color = mix(motion_color, color, texture2D(sampler_04, v_texcoord).r);
    }

    vec4 bloom = texture2D(sampler_05, v_texcoord);
    float ao = texture2D(sampler_02, v_texcoord).r;
    vec4 emissive = texture2D(sampler_03, v_texcoord);

    bloom.rgb = adjust_saturation(bloom.rgb, bloom_saturation) * bloom_intensity;
    color.rgb = adjust_saturation(color.rgb, original_saturation) * original_intensity;
    color *= ao;
    color += bloom;
    color += emissive;

    if (parameters_01.y > 0.0)
    {
        vec3 sepia_color;
        sepia_color.r = dot(color.rgb, vec3(0.393, 0.769, 0.189));
        sepia_color.g = dot(color.rgb, vec3(0.349, 0.686, 0.168));
        sepia_color.b = dot(color.rgb, vec3(0.272, 0.534, 0.131));
        float mask = texture2D(sampler_04, v_texcoord).r;
        color.rgb = mix(color.rgb, sepia_color, (1.0 - mask) * parameters_01.y);
    }

    float edge = 0.1;
    vec2 direction_to_edge = max(abs(v_texcoord * 2.0 - 1.0) * (1.0 + edge) - edge + parameters_01.x, vec2(0.0));
    float vignetting_value = dot(direction_to_edge, direction_to_edge);
    vignetting_value = 1.0 - vignetting_value / (1.0 + vignetting_value);
    color.rgb += (1.0 - vignetting_value) * vignetting_color.rgb;

    gl_FragColor = color;
}
