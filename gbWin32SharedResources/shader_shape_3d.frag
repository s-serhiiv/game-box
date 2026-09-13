layout(location = 1) out vec4 attachment_02;
layout(location = 2) out vec4 attachment_03;

void main()
{
    vec4 color = texture2D(sampler_01, v_texcoord);
    attachment_01 = color;
    attachment_02 = vec4(normalize(v_normal), 1.0);
    attachment_03 = vec4(v_screen_position.xyz, gl_FragCoord.z);
}
