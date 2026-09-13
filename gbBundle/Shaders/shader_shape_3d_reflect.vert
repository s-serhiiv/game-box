void main()
{
    gl_Position = get_pos_mvp();
    v_position = get_mat_m() * vec4(a_position, 1.0);
    v_screen_position = get_mat_v() * v_position;
    v_texcoord = a_texcoord;
	v_normal = normalize((get_mat_n() * vec4(a_normal.xyz, 0.0)).xyz);
	v_tangent = normalize((get_mat_n() * vec4(a_tangent.xyz, 0.0)).xyz);
    v_binormal = normalize(cross(-v_normal, v_tangent));
    v_color = vec4(normalize(-v_screen_position.xyz), 1.0);
    v_mat_tbn = mat3(normalize((get_mat_v() * get_mat_m() * vec4(a_normal.xyz, 0.0)).xyz), vec3(0.0), vec3(0.0));
}
