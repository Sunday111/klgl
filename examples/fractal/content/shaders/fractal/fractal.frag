in vec4 Color;
out vec4 FragColor;

uniform mat3 u_screen_to_world;
uniform vec2 u_julia_constant;
uniform vec3 uColorTable[MAX_ITERATIONS + 1];

void main()
{
    // Julia constant 
    vec2 c = u_julia_constant;

    vec2 world = (u_screen_to_world * vec3(gl_FragCoord.xy, 1)).xy; 
    vec2 z = world;

    // Julia set iteration
    int i = 0;
    while (i != MAX_ITERATIONS) {
        vec2 p = c + vec2(
            z.x * z.x - z.y * z.y,
            2 * z.x * z.y);

        if (dot(p, p) > 4) break;
        z = p;
        ++i;
    }

    // float pp = MAX_ITERATIONS * dot(normalize(world), vec2(1, 0));
    // FragColor = vec4(uColorTable[(int(pp) + i) % MAX_ITERATIONS], 1.0);

    FragColor = vec4(uColorTable[i], 1.0);
}
