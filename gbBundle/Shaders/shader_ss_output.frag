void main()
{
    vec4 color = texture2D(sampler_01, v_texcoord);
#if defined(__WINOS__)
    color.rgb = pow(max(color.rgb, vec3(0.0)), vec3(1.0 / 2.2));
#endif
    gl_FragColor = color;
}
