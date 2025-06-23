out vec4 FragColor;

uniform mat3 u_screen_to_world;
uniform vec2 u_resolution;
uniform vec2 u_julia_constant;
uniform vec3 uColorTable[MAX_ITERATIONS + 1];

#if INSIDE_OUT_SPACE
vec2 insideOutWarp(vec2 pos, vec2 center, float strength) {
    vec2 dir = pos - center;
    float dist = length(dir);
    
    // Avoid division by zero
    if (dist < 0.001) return pos;

    // Inversion formula: radius squared over distance
    float invertedDist = strength / dist;

    return center + normalize(dir) * invertedDist;
}
#endif

vec2 screen_point_to_world(vec2 screen) {
    return (u_screen_to_world * vec3(screen, 1)).xy;
}

vec2 complexMult(vec2 a, vec2 b) {
    return vec2(
        a.x * b.x - a.y * b.y,
        a.x * b.y + a.y * b.x
    );
}

void main()
{
    // Julia constant 
    vec2 c = u_julia_constant;

    vec2 world = screen_point_to_world(gl_FragCoord.xy);

#if INSIDE_OUT_SPACE
    vec2 world_center = screen_point_to_world(u_resolution / 2);
    vec2 z = insideOutWarp(world, world_center, 1);
#else
    vec2 z = world;
#endif

    // Julia set iteration
    int i = 0;
    while (i != MAX_ITERATIONS) {
        vec2 p = z;

        #pragma unroll
        for (int j = 1; j < COMPLEX_POWER; ++j) {
            p = complexMult(p, z);
        }

        p += c;

        if (dot(p, p) > 4) break;
        z = p;
        ++i;
    }

#if COLOR_MODE == 1
    float pp = MAX_ITERATIONS * dot(normalize(world), vec2(1, 0));
    FragColor = vec4(uColorTable[(int(pp) + i) % MAX_ITERATIONS], 1.0);
#elif COLOR_MODE == 2
    FragColor = vec4(uColorTable[(int(MAX_ITERATIONS * (world.x + world.y)) + i) % MAX_ITERATIONS], 1.0);
#else
    FragColor = vec4(uColorTable[i], 1.0);
#endif
}
