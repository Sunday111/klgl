in vec4 gs_color;
flat in uint gs_type;
in vec2 gs_tex_coord;
out vec4 FragColor;

void main()
{
    vec4 color;

    bool is_border =
        gs_tex_coord.x < 0.05f || gs_tex_coord.x > 0.95f || gs_tex_coord.y < 0.05f || gs_tex_coord.y > 0.95f;

    switch (gs_type)
    {
    case 0u:
        color = gs_color;
        break;

    case 1u:
        vec2 v = gs_tex_coord * 2 - 1;
        float dist_sq = dot(v, v);
        bool in_circle = dist_sq <= 1.f;
        is_border = in_circle && dist_sq >= (0.95f * 0.95f);
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
