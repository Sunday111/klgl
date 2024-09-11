layout(location = 0) in vec2 vertex_attribute;
layout(location = 1) in vec2 tex_coord_attribute;

uniform vec4 u_color;
uniform vec2 u_translation;
uniform vec2 u_scale;

out vec4 Color;
out vec2 TexCoord;

void main()
{
    gl_Position = vec4(vertex_attribute * u_scale + u_translation, 0.0, 1.0);
    Color = u_color;
    TexCoord = tex_coord_attribute;
}
