layout(location = 0) in vec2 a_vertex;
layout(location = 1) in uint u_type;
layout(location = 2) in vec4 u_color;
layout(location = 4) in mat3 u_transform;

flat out uint shape_type;
flat out vec4 shape_color;
out vec2 tex_coord;

void main()
{
    // Move the right top corner of quad to the bottom left corner
    // in case the figure is a triangle making it occupy zero area
    // is it will not be rasterized
    vec2 vertex = (u_type == 2u && (a_vertex.x + a_vertex.y) > 1.5f) ? vec2(1, -1) : a_vertex;
    gl_Position = vec4(u_transform * vec3(vertex, 1), 1.0);

    shape_type = u_type;
    shape_color = u_color;
    tex_coord = (a_vertex + 1) / 2;
}
