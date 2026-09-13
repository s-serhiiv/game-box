void main()
{
    float emissive = texture2D(sampler_01, v_texcoord).r;
    gl_FragColor = vec4(v_color.rgb, emissive * v_color.a);
}
