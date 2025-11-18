out vec4 FragColor;

uniform mat3 u_screen_to_world;
uniform vec2 u_resolution;
uniform vec2 u_julia_constant;
uniform vec3 uColorTable[MAX_ITERATIONS + 1];
uniform vec2 u_fractal_power;

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

vec2 complexExp(vec2 z)
{
    return exp(z.x) * vec2(cos(z.y), sin(z.y));
}

vec2 complexPower(vec2 base, vec2 power) {
    float r = length(base);
    float theta = atan(base.x, base.y);
    vec2 log_z = vec2(log(r), theta);
    vec2 exponent = complexMult(power, log_z);
    return complexExp(exponent);
}

vec3 getColor(float smoothIteration) {
    float index = clamp(smoothIteration, 0.0, float(MAX_ITERATIONS));
    int lower = int(floor(index));
    int upper = min(lower + 1, MAX_ITERATIONS);

    float t = index - float(lower); // fractional part

    vec3 colorLower = uColorTable[lower];
    vec3 colorUpper = uColorTable[upper];

    return mix(colorLower, colorUpper, t);
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
        vec2 p = complexPower(z, u_fractal_power) + c;
        if (dot(p, p) > 4) break;
        z = p;
        ++i;
    }

#if COLOR_MODE == 1
    float pp = MAX_ITERATIONS * dot(normalize(world), vec2(1, 0));
    FragColor = vec4(uColorTable[(int(pp) + i) % MAX_ITERATIONS], 1.0);
#elif COLOR_MODE == 2
    FragColor = vec4(uColorTable[(int(MAX_ITERATIONS * (world.x + world.y)) + i) % MAX_ITERATIONS], 1.0);
#elif COLOR_MODE == 3
    float smooth_iteration = float(i) - log2(max(1.0f, log2(sqrt(length(z)))));
    float maxIterF = float(MAX_ITERATIONS);
    vec3 color = getColor(smooth_iteration);
    FragColor = vec4(color, 1.0);
#elif COLOR_MODE == 4
    float smooth_iteration = float(i) - log2(log2(length(z)));
    float maxIterF = float(MAX_ITERATIONS);
    vec3 color = getColor(smooth_iteration);
    FragColor = vec4(color, 1.0);
#else
    FragColor = vec4(uColorTable[i], 1.0);
#endif
}
