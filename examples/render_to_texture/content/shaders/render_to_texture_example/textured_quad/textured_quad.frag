in vec4 Color;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D u_texture;

void main()
{
    FragColor = Color * texture(u_texture, TexCoord);
}
