layout(location = 0) in vec2 a_vertex;
layout(location = 1) in vec4 a_color;

uniform mat3 u_transform;

out vec4 vs_color;

void main()
{
    gl_Position = vec4(u_transform * vec3(a_vertex, 1), 1.0);
    vs_color = a_color;
}
