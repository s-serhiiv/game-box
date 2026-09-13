void main()
{
    gl_Position = get_mat_p() * get_mat_v() * vec4(a_position, 1.0);
    v_texcoord = a_texcoord;
    v_color = a_color;
}
