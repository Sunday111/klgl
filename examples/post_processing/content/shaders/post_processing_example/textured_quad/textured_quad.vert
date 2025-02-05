layout(location = 0) in vec2 a_vertex;
layout(location = 1) in vec2 a_tex_coord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(a_vertex, 0.0, 1.0);
    TexCoord = a_tex_coord;
}
