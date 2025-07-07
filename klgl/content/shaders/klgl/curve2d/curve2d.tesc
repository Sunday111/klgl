layout(vertices = 4) out;
uniform vec2 u_viewport_size;
const float k_max_tess_level = 64.0;
const float k_segment_pixel_length = 8.0f;
in vec4 vs_color[];
out vec4 tc_color[];
patch out float tc_segments;
void main() {
    if (gl_InvocationID == 0) {
        vec4 p1 = gl_in[1].gl_Position;
        vec4 p2 = gl_in[2].gl_Position;
        vec2 ndc1 = p1.xy / p1.w;
        vec2 ndc2 = p2.xy / p2.w;
        vec2 screen1 = (ndc1 * 0.5 + 0.5) * u_viewport_size;
        vec2 screen2 = (ndc2 * 0.5 + 0.5) * u_viewport_size;
        float length_pixels = length(screen2 - screen1);
        tc_segments = clamp(length_pixels / k_segment_pixel_length, 1.0, k_max_tess_level);
        gl_TessLevelOuter[0] = 1.0;
        gl_TessLevelOuter[1] = tc_segments;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tc_color[gl_InvocationID] = vs_color[gl_InvocationID];
}
