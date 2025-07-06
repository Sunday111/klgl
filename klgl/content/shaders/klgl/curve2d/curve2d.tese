layout(isolines, fractional_even_spacing, cw) in;
in vec4 tc_color[];
out vec4 te_color;
vec4 catmull_rom_vec4(vec4 p0, vec4 p1, vec4 p2, vec4 p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5 * (2.0 * p1 + (-p0 + p2) * t
        + (2.0*p0 - 5.0*p1 + 4.0*p2 - p3) * t2
        + (-p0 + 3.0*p1 - 3.0*p2 + p3) * t3);
}
vec3 catmull_rom_vec3(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5 * (2.0 * p1 + (-p0 + p2) * t
        + (2.0*p0 - 5.0*p1 + 4.0*p2 - p3) * t2
        + (-p0 + 3.0*p1 - 3.0*p2 + p3) * t3);
}
void main() {
    float t = gl_TessCoord.x;
    te_color = catmull_rom_vec4(tc_color[0], tc_color[1], tc_color[2], tc_color[3], t);
    vec3 pos = catmull_rom_vec3(
        gl_in[0].gl_Position.xyz,
        gl_in[1].gl_Position.xyz,
        gl_in[2].gl_Position.xyz,
        gl_in[3].gl_Position.xyz,
        t
    );
    gl_Position = vec4(pos, 1.0);
}
