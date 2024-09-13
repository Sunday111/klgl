in vec4 gs_color;
flat in uint gs_type;
in vec2 gs_tex_coord;
out vec4 FragColor;

void main()
{
    vec4 color;

    const float border_left = FIGURE_BORDER;
    const float border_right = 1.f - border_left;

    bool is_border = gs_tex_coord.x < border_left || gs_tex_coord.x > border_right || gs_tex_coord.y < border_left ||
                     gs_tex_coord.y > border_right;

    switch (gs_type)
    {
    case 0u:
        color = gs_color;
        break;

    case 1u:
        vec2 v = gs_tex_coord * 2 - 1;
        float dist_sq = dot(v, v);
        bool in_circle = dist_sq <= 1.f;
        is_border = in_circle && dist_sq >= border_right * border_right;
        color = in_circle ? gs_color : vec4(0);
        break;

    case 2u:
        color = gs_color;
        break;

    default:
        break;
    }

    color.xyz = is_border ? 1 - color.xyz : color.xyz;
    FragColor = color;
}
