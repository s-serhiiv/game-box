void main()
{
    vec3 hdr_color = texture2D(sampler_01, v_texcoord).rgb;
    const vec3 LUM_FACTOR = vec3(0.299, 0.587, 0.114);
    float l_scale = dot(hdr_color, LUM_FACTOR);
    hdr_color *= l_scale;
    gl_FragColor = vec4(hdr_color, 1.0);
}
