layout(location = 0) in vec2 vertex_attribute;

uniform vec4 u_color;
uniform mat3 u_transform;

out vec4 Color;

void main()
{
    gl_Position = vec4(u_transform * vec3(vertex_attribute, 1), 1.0);
    Color = u_color;
}
