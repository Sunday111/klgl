in vec2 uv;
out vec4 FragColor;

uniform vec3 uColorTable[COLORS_COUNT];

void main()
{
    FragColor = vec4(uColorTable[int(clamp(uv.x * (COLORS_COUNT - 1), 0, COLORS_COUNT - 1))], 1);
}
