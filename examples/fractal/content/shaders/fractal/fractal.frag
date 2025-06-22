in vec4 Color;
out vec4 FragColor;

uniform mat3 u_screen_to_world;
uniform vec2 u_resolution;
uniform vec2 u_julia_constant;
uniform vec3 uColorTable[MAX_ITERATIONS + 1];

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(
        abs(q.z + (q.w - q.y) / (6.0 * d + e)), // Hue
        d / (q.x + e),                          // Saturation
        q.x                                     // Value
    );
}

vec3 hsv2rgb(vec3 c) {
    vec3 rgb = clamp(
        abs(mod(c.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0,
        0.0,
        1.0
    );
    return c.z * mix(vec3(1.0), rgb, c.y);
}

vec3 hsvMix(vec3 hsv1, vec3 hsv2, float t) {
    float h1 = hsv1.x;
    float h2 = hsv2.x;

    // Shortest path interpolation around hue circle
    if (abs(h1 - h2) > 0.5) {
        if (h1 > h2) {
            h2 += 1.0;
        } else {
            h1 += 1.0;
        }
    }

    float h = mix(h1, h2, t);
    h = mod(h, 1.0); // Wrap hue back into [0, 1]

    float s = mix(hsv1.y, hsv2.y, t);
    float v = mix(hsv1.z, hsv2.z, t);

    return vec3(h, s, v);
}

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
        vec2 p = c + vec2(
            z.x * z.x - z.y * z.y,
            2 * z.x * z.y);

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
    if (i == MAX_ITERATIONS) {
        FragColor = vec4(uColorTable[i], 1.0);
    } else
    {
        float t = clamp(1.0 - log2(log2(length(z))), 0, 1);
        vec3 a = uColorTable[i];
        vec3 b = uColorTable[i + 1];
        FragColor = vec4(hsv2rgb(hsvMix(rgb2hsv(a), rgb2hsv(b), t)), 1);
    }
#else
    FragColor = vec4(uColorTable[i], 1.0);
#endif
}
