void main()
{
    float emissive = texture2D(sampler_01, v_texcoord).r;
    gl_FragColor = v_color * emissive;
}
