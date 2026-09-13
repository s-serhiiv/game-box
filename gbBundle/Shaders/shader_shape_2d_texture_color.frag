void main()
{
    vec4 color = vec4(v_color.rgb, 1.0);
    color.a = texture2D(sampler_01, v_texcoord).a * v_color.a;
    gl_FragColor = color;
}
