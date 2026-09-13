void main()
{
    gl_Position = get_mat_p() * get_mat_v() * vec4(a_position, 1.0);
    v_position = vec4(a_position, 1.0);
    v_texcoord = a_texcoord;
    v_color = a_color;
    v_normal = normalize(a_normal.xyz);
    v_tangent = normalize(a_tangent.xyz);
    v_binormal = normalize(cross(-v_normal, v_tangent));
}
