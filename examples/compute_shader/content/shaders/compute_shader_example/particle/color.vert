layout(location = 0) in vec3 vertex_position;

uniform vec4 u_color;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec4 Color;

void main()
{
    gl_Position = u_projection * (u_view * (u_model * vec4(vertex_position, 1)));
    Color = u_color;
}
