void main()
{
    vec4 color = v_color;
    color.a = texture2D(sampler_01, v_texcoord).a;
    gl_FragColor = color;
}
