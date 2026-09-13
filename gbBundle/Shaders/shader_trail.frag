void main()
{
    float depth = texelFetch(sampler_02, ivec2(gl_FragCoord.xy), 0).w;
    vec4 color = vec4(v_color.rgb * -1.0, 0.0);
    color.a = texture2D(sampler_01, v_texcoord).a * v_color.a;
    float weight = (depth - gl_FragCoord.w) / 1.66;
    gl_FragColor = vec4(color.rgb * color.a, color.a * weight);
}
