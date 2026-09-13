uniform mat4  u_mat_i_vp;
uniform vec3  u_position;
uniform float u_radius;
uniform vec4  u_color;

void main()
{
    vec4 screen_position = v_screen_position;
    screen_position.xy /= screen_position.w;
    vec2 texcoord = 0.5 * (screen_position.xy + 1.0);
    
    vec4 color = texture2D(sampler_01, texcoord);
    float depth = texture2D(sampler_02, texcoord).r;
    
    vec4 position = vec4(texcoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    position = u_mat_i_vp * position;
    position.xyz = position.xyz / position.w;
    
    vec3 normal = color.xyz * 2.0 - 1.0;
    
    vec3 light_direction = u_position - position.xyz;
    float attenuation = 1.0 - length(light_direction) / u_radius;
    light_direction = normalize(light_direction);
    
    gl_FragColor = vec4(vec3(attenuation * clamp(dot(normal, light_direction), 0.0, 1.0)) * u_color.xyz, 1.0);
}
