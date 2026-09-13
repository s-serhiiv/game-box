void main()
{
    gl_Position = get_pos_mvp();
    v_texcoord = a_texcoord;
    v_screen_position = get_mat_m() * vec4(a_position, 1.0);
	vec4 normal = vec4(a_normal.xyz, 0.0);
	v_normal = normalize((get_mat_n() * normal).xyz);
    v_color = a_color;
}
