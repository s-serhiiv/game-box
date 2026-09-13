void main()
{
    vec4 opaque_color = texture2D(sampler_01, v_texcoord);
    vec4 transparent_color = texture2D(sampler_02, v_texcoord);
    vec4 color = opaque_color + transparent_color;
    vec4 lighting = texture2D(sampler_03, v_texcoord);
    vec4 mask = texture2D(sampler_04, v_texcoord);
    color.rgb *= clamp(lighting.rgb, vec3(mask.r * 0.15), vec3(0.999));
    gl_FragColor = color;
}

