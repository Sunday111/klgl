out vec4 FragColor;

uniform vec2 u_resolution;
uniform vec3 u_color_table[MAX_ITERATIONS + 1];

layout(std430, binding = 0) buffer PixelBuffer {
    uint visitCounts[];
};

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    int index = pixel.y * int(u_resolution.x) + pixel.x;
    uint count = visitCounts[index];
    FragColor = vec4(u_color_table[count], 1);
}
