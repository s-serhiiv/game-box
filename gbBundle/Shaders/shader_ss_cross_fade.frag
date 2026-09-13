uniform float enabled;
uniform float progress;

float rand(vec2 value)
{
    return fract(sin(dot(value, vec2(12.9898, 78.233))) * 43758.5453);
}

float linear_ease(float start, float change, float duration, float time)
{
    return change * time / duration + start;
}

float sinusoidal_ease_in_out(float start, float change, float duration, float time)
{
    return -change * 0.5 * (cos(3.14159265 * time / duration) - 1.0) + start;
}

void main()
{
    vec2 uv = v_texcoord;
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

    if (enabled > 0.0)
    {
        vec2 center = vec2(linear_ease(0.5, 0.0, 1.0, progress), 0.5);
        float strength = sinusoidal_ease_in_out(0.0, 1.0, 1.0, progress);
        float max_weight = 0.0;
        vec2 to_center = center - uv;
        float offset = rand(vec2(uv.y * progress));

        for (float i = 0.0; i <= 20.0; i += 1.0)
        {
            float percent = (i + offset) / 20.0;
            float weight = percent - percent * percent;
            color += mix(texture2D(sampler_01, uv + to_center * percent * strength), vec4(0.0, 0.0, 0.0, 1.0), progress) * weight;
            max_weight += weight;
        }

        color /= max_weight;
        color.a = 1.0;
    }
    else
    {
        color = texture2D(sampler_01, uv);
    }

    gl_FragColor = color;
}
