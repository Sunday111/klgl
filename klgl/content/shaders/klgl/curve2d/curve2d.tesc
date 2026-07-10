layout(vertices = 6) out;
uniform vec2 u_viewport_size;
const float k_max_tess_level = 64.0;
uniform float u_segment_pixel_length;
in vec4 vs_color[];
out vec4 tc_color[];
patch out float tc_segments;
patch out float tc_prev_segments;
patch out float tc_next_segments;

float segment_count(vec4 p1, vec4 p2) {
    vec2 ndc1 = p1.xy / p1.w;
    vec2 ndc2 = p2.xy / p2.w;
    vec2 screen1 = (ndc1 * 0.5 + 0.5) * u_viewport_size;
    vec2 screen2 = (ndc2 * 0.5 + 0.5) * u_viewport_size;
    float requested = clamp(length(screen2 - screen1) / u_segment_pixel_length, 1.0, k_max_tess_level);
    return 2.0 * ceil(requested * 0.5);
}

void main() {
    if (gl_InvocationID == 0) {
        tc_prev_segments = segment_count(gl_in[1].gl_Position, gl_in[2].gl_Position);
        tc_segments = segment_count(gl_in[2].gl_Position, gl_in[3].gl_Position);
        tc_next_segments = segment_count(gl_in[3].gl_Position, gl_in[4].gl_Position);
        gl_TessLevelOuter[0] = 1.0;
        gl_TessLevelOuter[1] = tc_segments;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tc_color[gl_InvocationID] = vs_color[gl_InvocationID];
}
