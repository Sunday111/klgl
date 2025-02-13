flat in uint gs_type;
flat in vec4 gs_color;
in vec2 gs_tex_coord;

out vec4 FragColor;

void main()
{
    switch (gs_type)
    {
    // Rectangle
    case 0u:
        FragColor = gs_color;
        break;

    // Circle (ellipse)
    case 1u:
        vec2 cc = gs_tex_coord * 2 - 1;
        FragColor = dot(cc, cc) <= 1 ? gs_color : vec4(0, 0, 0, 0);
        break;

    // Triangle
    case 2u:
        FragColor = gs_color;
        break;

    // Rectangle lines
    case 3u:
        FragColor = gs_color;
        break;

    // Triangle lines
    case 4u:
        FragColor = gs_color;
        break;

    // Draw yellow grid pattern to show bad area
    default:
        int k = int(gs_tex_coord.x * 10) + int(gs_tex_coord.y * 10);
        FragColor = (k % 2 == 0) ? vec4(1, 1, 0, 1) : vec4(0, 0, 0, 0);
        break;
    }
}
