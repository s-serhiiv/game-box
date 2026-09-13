void main()
{
    float alpha = texture2D(sampler_01, v_texcoord).r;
    gl_FragColor = vec4(vec3(4.0), alpha);
}
