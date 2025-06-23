layout(location = 0) in vec2 vertex_attribute;

void main()
{
    gl_Position = vec4(vertex_attribute, 1, 1.0);
}
