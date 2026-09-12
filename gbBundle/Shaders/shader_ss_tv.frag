uniform float enabled;
uniform float time;

void main()
{
    vec2 uv = v_texcoord;

    if (enabled > 0.0)
    {
        float effect_time = time * 0.001;
        float distance_to_center = length(uv - vec2(0.5));

        float blur = (1.0 + sin(effect_time * 6.0)) * 0.5;
        blur *= 1.0 + sin(effect_time * 16.0) * 0.5;
        blur = pow(blur, 3.0);
        blur *= 0.05 * distance_to_center;

        vec3 color;
        color.r = texture2D(sampler_01, vec2(uv.x + blur, uv.y)).r;
        color.g = texture2D(sampler_01, uv).g;
        color.b = texture2D(sampler_01, vec2(uv.x - blur, uv.y)).b;

        float scanline = sin(uv.y * 768.0) * 0.04;
        color -= vec3(scanline);
        color *= 1.0 - distance_to_center * 0.5;

        gl_FragColor = vec4(color, 1.0);
    }
    else
    {
        gl_FragColor = texture2D(sampler_01, uv);
    }
}
