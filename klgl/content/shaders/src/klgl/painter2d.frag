flat in uint shape_type;
flat in vec4 shape_color;
in vec2 tex_coord;

out vec4 FragColor;

void main()
{
    switch (shape_type)
    {
    // Rectangle
    case 0u:
        FragColor = shape_color;
        break;

    // Circle (ellipse)
    case 1u:
        vec2 cc = tex_coord * 2 - 1;
        FragColor = dot(cc, cc) <= 1 ? shape_color : vec4(0, 0, 0, 0);
        break;

    // Triangle
    case 2u:
        FragColor = shape_color;
        break;

    // Draw yellow grid pattern to show bad area
    default:
        int k = int(tex_coord.x * 10) + int(tex_coord.y * 10);
        FragColor = (k % 2 == 0) ? vec4(1, 1, 0, 1) : vec4(0, 0, 0, 0);
        break;
    }
}
