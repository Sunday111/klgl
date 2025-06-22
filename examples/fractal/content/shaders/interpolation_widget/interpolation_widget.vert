layout(location = 0) in vec2 vertex_attribute;

out vec2 uv;

void main()
{
    gl_Position = vec4(vertex_attribute, 1, 1.0);
    uv = (vertex_attribute + 1) / 2;
}
