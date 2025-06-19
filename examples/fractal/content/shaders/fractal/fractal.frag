in vec4 Color;
out vec4 FragColor;


uniform mat3 u_screen_to_world;
uniform vec2 u_julia_constant;

void main()
{
    // Julia constant 
    vec2 c = u_julia_constant;

    // Map to complex plane
    vec2 z = (u_screen_to_world * vec3(gl_FragCoord.xy, 1)).xy;

    // Julia set iteration
    int maxIter = 200;
    int i;
    for (i = 0; i < maxIter; i++) {
        float x = (z.x * z.x - z.y * z.y) + c.x;
        float y = (2.0 * z.x * z.y) + c.y;

        if (x * x + y * y > 4.0) break;
        z = vec2(x, y);
    }

    // Smooth coloring
    float t = float(i) / float(maxIter);
    vec3 color = vec3(0.5 + 0.5 * cos(6.2831 * t + vec3(0.0, 0.33, 0.67)));

    FragColor = vec4(color, 1.0);
}
