layout(isolines, fractional_even_spacing, cw) in;
in vec4 tc_color[];
patch in float tc_segments;
patch in float tc_prev_segments;
patch in float tc_next_segments;
out vec4 te_color;
out vec4 te_join_normals;
out vec4 te_join_vertices;
uniform float u_thickness;
uniform vec2 u_viewport_size;

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

vec2 rotate_ccw90(vec2 v) {
    return vec2(-v.y, v.x);
}

void calculate_join(vec2 previous, vec2 current, vec2 next) {
    vec2 screen_scale = u_viewport_size * 0.5;
    vec2 previous_delta = (current - previous) * screen_scale;
    vec2 current_delta = (next - current) * screen_scale;
    float previous_length_sq = dot(previous_delta, previous_delta);
    float current_length_sq = dot(current_delta, current_delta);
    vec2 outgoing_dir = current_length_sq < 1e-6
        ? (previous_length_sq < 1e-6 ? vec2(1, 0) : normalize(previous_delta))
        : normalize(current_delta);
    vec2 incoming_dir = previous_length_sq < 1e-6 ? outgoing_dir : normalize(previous_delta);

    float half_width = u_thickness * 0.5;
    vec2 k1 = rotate_ccw90(incoming_dir);
    vec2 k2 = rotate_ccw90(outgoing_dir);
    vec2 n1 = half_width * k1;
    vec2 n2 = half_width * k2;
    vec2 normal_sum = k1 + k2;
    float normal_sum_sq = dot(normal_sum, normal_sum);

    vec2 miter;
    vec2 bisector;
    float denominator = dot(normal_sum, k1);
    if (normal_sum_sq < 1e-6 || abs(denominator) < 1e-3) {
        miter = n1;
        bisector = n1;
    } else {
        miter = half_width * normal_sum / denominator;
        bisector = half_width * normal_sum * inversesqrt(normal_sum_sq);

        float max_miter_length = half_width * 4.0;
        float miter_length_sq = dot(miter, miter);
        if (miter_length_sq > max_miter_length * max_miter_length) {
            miter *= max_miter_length * inversesqrt(miter_length_sq);
        }
    }

    float turn = incoming_dir.x * outgoing_dir.y - incoming_dir.y * outgoing_dir.x;
    vec2 v1 = turn < 0.0 ? -miter : -bisector;
    vec2 v2 = turn < 0.0 ? bisector : miter;

    te_join_normals = vec4(n1, n2);
    te_join_vertices = vec4(v1, v2);
    vec2 clip_scale = 2.0 / u_viewport_size;
    vec4 packed_clip_scale = vec4(clip_scale, clip_scale);
    te_join_normals *= packed_clip_scale;
    te_join_vertices *= packed_clip_scale;
}

void main() {
    float t = gl_TessCoord.x;

    te_color = catmull_rom_vec4(tc_color[1], tc_color[2], tc_color[3], tc_color[4], t);
    vec2 current_point = catmull_rom_vec2(
            gl_in[1].gl_Position.xy,
            gl_in[2].gl_Position.xy,
            gl_in[3].gl_Position.xy,
            gl_in[4].gl_Position.xy,
            t
        );
    gl_Position = vec4(current_point, 0, 1);

    vec2 previous_point;
    if (t <= 0.0) {
        previous_point = catmull_rom_vec2(
            gl_in[0].gl_Position.xy, gl_in[1].gl_Position.xy,
            gl_in[2].gl_Position.xy, gl_in[3].gl_Position.xy,
            1.0 - 1.0 / tc_prev_segments);
    } else {
        previous_point = catmull_rom_vec2(
            gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy,
            gl_in[3].gl_Position.xy, gl_in[4].gl_Position.xy,
            max(t - 1.0 / tc_segments, 0.0));
    }

    vec2 next_point;
    if (t >= 1.0) {
        next_point = catmull_rom_vec2(
            gl_in[2].gl_Position.xy, gl_in[3].gl_Position.xy,
            gl_in[4].gl_Position.xy, gl_in[5].gl_Position.xy,
            1.0 / tc_next_segments);
    } else {
        next_point = catmull_rom_vec2(
            gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy,
            gl_in[3].gl_Position.xy, gl_in[4].gl_Position.xy,
            min(t + 1.0 / tc_segments, 1.0));
    }

    calculate_join(previous_point, current_point, next_point);
}
