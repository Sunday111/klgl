in vec2 a_vertex;
in uint a_type;
in vec4 a_color;
in mat3 a_transform;

flat out uint shape_type;
flat out vec4 shape_color;
out vec2 tex_coord;

void main()
{
    // Move the right top corner of quad to the bottom left corner
    // in case the figure is a triangle making it occupy zero area
    // is it will not be rasterized
    vec2 vertex = (a_type == 2u && (a_vertex.x + a_vertex.y) > 1.5f) ? vec2(1, -1) : a_vertex;
    gl_Position = vec4(a_transform * vec3(vertex, 1), 1.0);

    shape_type = a_type;
    shape_color = a_color;
    tex_coord = (a_vertex + 1) / 2;
}
