uniform vec4 body_color;
uniform vec4 windows_color;

// GB_SAMPLER_02_CUBE

layout(location = 1) out vec4 attachment_02;
layout(location = 2) out vec4 attachment_03;

void main()
{
    vec3 reflection_direction = reflect(v_color.xyz, normalize(v_mat_tbn[0]));
    vec4 reflection_color = texture(sampler_02, reflection_direction);
    vec4 color = texture2D(sampler_01, v_texcoord);
    vec4 body_color_mask = texture2D(sampler_03, v_texcoord);
    if (body_color_mask.a > 0.5)
    {
        color.rgb *= body_color.rgb * body_color_mask.a;
    }

    vec4 windows_color_mask = texture2D(sampler_04, v_texcoord);
    color.rgb = mix(color.rgb, windows_color.rgb, windows_color_mask.a);
    color.a = 1.0;

    vec3 normal_color = normalize(texture2D(sampler_05, v_texcoord).xyz * 2.0 - 1.0);
    vec3 normal = normalize(normal_color.x * v_tangent + normal_color.y * v_binormal + normal_color.z * v_normal);

    attachment_01 = mix(pow(reflection_color, vec4(2.0)), color, vec4(0.66) - windows_color_mask * 0.33);
    attachment_01.a = 1.0;
    attachment_02 = vec4(normal, 0.0);
    attachment_03 = vec4(v_position.xyz, gl_FragCoord.z);

    float rim = smoothstep(0.7, 1.0, 1.0 - max(dot(v_color.xyz, normalize(v_mat_tbn[0])), 0.0));
    attachment_01.rgb += rim * attachment_01.rgb * 8.0;
}
