in vec4 Color;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D u_texture_a;
uniform sampler2D u_texture_b;

void main()
{
    FragColor = Color * texture(u_texture_a, TexCoord).r * texture(u_texture_b, TexCoord).r;
}
