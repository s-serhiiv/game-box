uniform vec3 u_light_position;
uniform float u_light_ray_length;
uniform vec4 u_light_color;
uniform vec4 u_camera_position;

void main()
{
    ivec2 screen_position = ivec2(gl_FragCoord.xy);
    vec4 normal = texelFetch(sampler_01, screen_position, 0);
    vec4 position = texelFetch(sampler_02, screen_position, 0);

    vec3 light_vector = u_light_position - position.xyz;
    float light_distance = length(light_vector);
    vec4 color = vec4(0.0);
    if (light_distance < u_light_ray_length)
    {
        vec3 light_direction = normalize(light_vector);
        float intensity = max(dot(light_direction, normal.xyz), 0.0);
        float attenuation = 1.0 - light_distance / u_light_ray_length;
        attenuation *= attenuation;
        vec3 reflection_vector = normalize(reflect(-light_direction, normal.xyz));
        vec3 camera_direction = normalize(u_camera_position.xyz - position.xyz);
        vec4 specular = u_light_color * intensity * pow(max(dot(reflection_vector, camera_direction), 0.0), 16.0);
        specular *= attenuation * normal.w;
        color = (u_light_color * intensity + specular) * attenuation;
    }
    gl_FragColor = color;
}
