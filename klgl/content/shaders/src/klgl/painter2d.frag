flat in int shape_type;
in vec4 shape_color;
in vec2 tex_coord;

out vec4 FragColor;

void main()
{
    FragColor = vec4(1, 0, 0, 1);

    switch (shape_type)
    {
    case 0:
        FragColor = shape_color;
        break;
    case 1:
        vec2 cc = tex_coord * 2 - 1;
        if (dot(cc, cc) <= 1)
        {
            FragColor = shape_color;
        }
        else
        {
            FragColor = vec4(0, 0, 0, 0);
        }
        break;
    }
}
