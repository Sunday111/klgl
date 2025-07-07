layout(isolines, fractional_even_spacing, cw) in;
in vec4 tc_color[];
patch in float tc_segments;
out vec4 te_color;
out vec2 te_prev_point;
out vec2 te_next_point;

vec4 catmull_rom_vec4(vec4 p0, vec4 p1, vec4 p2, vec4 p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5 * (2.0 * p1 + (-p0 + p2) * t
        + (2.0*p0 - 5.0*p1 + 4.0*p2 - p3) * t2
        + (-p0 + 3.0*p1 - 3.0*p2 + p3) * t3);
}
vec2 catmull_rom_vec2(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5 * (2.0 * p1 + (-p0 + p2) * t
        + (2.0*p0 - 5.0*p1 + 4.0*p2 - p3) * t2
        + (-p0 + 3.0*p1 - 3.0*p2 + p3) * t3);
}
void main() {
    float t = gl_TessCoord.x;
    float dt = 1.0 / tc_segments;
    float t_prev = t - dt;
    float t_next = t + dt;

    te_color = catmull_rom_vec4(tc_color[0], tc_color[1], tc_color[2], tc_color[3], t);
    gl_Position = vec4(catmull_rom_vec2(
            gl_in[0].gl_Position.xy,
            gl_in[1].gl_Position.xy,
            gl_in[2].gl_Position.xy,
            gl_in[3].gl_Position.xy,
            t
        ), 0, 1);

    vec2 at_prev_t = catmull_rom_vec2(
            gl_in[0].gl_Position.xy,
            gl_in[1].gl_Position.xy,
            gl_in[2].gl_Position.xy,
            gl_in[3].gl_Position.xy,
            max(0, t_prev)
        );

    vec2 at_next_t = catmull_rom_vec2(
            gl_in[0].gl_Position.xy,
            gl_in[1].gl_Position.xy,
            gl_in[2].gl_Position.xy,
            gl_in[3].gl_Position.xy,
            min(1, t_next)
        );

    te_prev_point = t_next >= 1 ? gl_in[3].gl_Position.xy : at_next_t;
}
