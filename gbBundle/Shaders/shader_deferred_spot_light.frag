uniform vec3 u_light_position;
uniform vec3 u_light_direction;
uniform float u_light_inner_cutoff_angle;
uniform float u_light_outer_cutoff_angle;
uniform vec4 u_light_color;
uniform vec4 u_camera_position;

void main()
{
    ivec2 screen_position = ivec2(gl_FragCoord.xy);
    vec4 normal = texelFetch(sampler_01, screen_position, 0);
    vec4 position = texelFetch(sampler_02, screen_position, 0);

    vec3 light_vector = u_light_position - position.xyz;
    vec3 light_direction = normalize(light_vector);
    float theta = dot(light_direction, normalize(-u_light_direction));
    float epsilon = u_light_inner_cutoff_angle - u_light_outer_cutoff_angle;
    float attenuation = max((theta - u_light_outer_cutoff_angle) / epsilon, 0.0);
    float intensity = max(dot(light_direction, normal.xyz), 0.0);

    vec4 color = u_light_color * intensity * attenuation;
    vec3 reflection_vector = normalize(reflect(-light_direction, normal.xyz));
    vec3 camera_direction = normalize(u_camera_position.xyz - position.xyz);
    vec4 specular = u_light_color * intensity * pow(max(dot(reflection_vector, camera_direction), 0.0), 16.0);
    specular *= attenuation * normal.w;
    gl_FragColor = color + specular;
}
