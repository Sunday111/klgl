layout(location = 0) in vec2 a_vertex;
layout(location = 1) in vec2 a_tex_coord;

uniform int u_type;
uniform vec4 u_color;
uniform mat3 u_transform;

flat out int shape_type;
out vec4 shape_color;
out vec2 tex_coord;

void main()
{
    gl_Position = vec4(u_transform * vec3(a_vertex, 1), 1.0);
    shape_type = u_type;
    shape_color = u_color;
    tex_coord = a_tex_coord;
}
