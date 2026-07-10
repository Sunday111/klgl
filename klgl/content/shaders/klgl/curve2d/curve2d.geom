layout(lines) in;
layout(triangle_strip,max_vertices=10) out;

in vec4 te_color[];
in vec2 te_prev_point[];
in vec2 te_next_point[];
out vec4 gs_color;
uniform float u_thickness;
uniform vec2 u_viewport_size;

vec2 rotate_ccw90(vec2 v) {
    return vec2(-v.y, v.x);
}

void calculate_join(
    vec2 incoming_dir,
    vec2 outgoing_dir,
    out vec2 incoming_plus,
    out vec2 incoming_minus,
    out vec2 outgoing_plus,
    out vec2 outgoing_minus,
    out vec2 v1,
    out vec2 v2,
    out float turn
) {
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
        // The model is singular at an exact reversal. Collapse to a bounded
        // bevel there rather than producing an unbounded spike.
        miter = n1;
        bisector = n1;
    } else {
        miter = half_width * normal_sum / denominator;
        bisector = half_width * normal_sum * inversesqrt(normal_sum_sq);

        // Preserve the exact construction except close to its 180-degree
        // singularity, where a finite miter is required for usable geometry.
        float miter_length = length(miter);
        float max_miter_length = half_width * 4.0;
        if (miter_length > max_miter_length) {
            miter *= max_miter_length / miter_length;
        }
    }

    turn = incoming_dir.x * outgoing_dir.y - incoming_dir.y * outgoing_dir.x;
    v1 = turn < 0.0 ? -miter : -bisector;
    v2 = turn < 0.0 ? bisector : miter;

    // Clip both segment cores at the shared V1/V2 construction. These are the
    // exact conditional polygon corners from the model.
    incoming_plus = turn < 0.0 ? n1 : v2;
    incoming_minus = turn < 0.0 ? v1 : -n1;
    outgoing_plus = turn < 0.0 ? n2 : v2;
    outgoing_minus = turn < 0.0 ? v1 : -n2;
}

void emit_quad(vec2 a_plus, vec2 b_plus, vec2 a_minus, vec2 b_minus, vec4 color_a, vec4 color_b) {
    gs_color = color_a; gl_Position = vec4(a_plus, 0, 1); EmitVertex();
    gs_color = color_b; gl_Position = vec4(b_plus, 0, 1); EmitVertex();
    gs_color = color_a; gl_Position = vec4(a_minus, 0, 1); EmitVertex();
    gs_color = color_b; gl_Position = vec4(b_minus, 0, 1); EmitVertex();
    EndPrimitive();
}

void emit_triangle(vec2 a, vec2 b, vec2 c, vec4 color) {
    gs_color = color; gl_Position = vec4(a, 0, 1); EmitVertex();
    gs_color = color; gl_Position = vec4(b, 0, 1); EmitVertex();
    gs_color = color; gl_Position = vec4(c, 0, 1); EmitVertex();
    EndPrimitive();
}

void main(){
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec2 s0 = (p0.xy + 1) * u_viewport_size / 2;
    vec2 s1 = (p1.xy + 1) * u_viewport_size / 2;
    vec2 sp = (te_prev_point[0] + 1) * u_viewport_size / 2;
    vec2 sn = (te_next_point[1] + 1) * u_viewport_size / 2;
    vec2 prev_delta = s0 - sp;
    vec2 curr_delta = s1 - s0;
    vec2 next_delta = sn - s1;
    float curr_length_sq = dot(curr_delta, curr_delta);
    vec2 dir_curr = curr_length_sq < 1e-6
        ? (dot(next_delta, next_delta) < 1e-6 ? vec2(1, 0) : normalize(next_delta))
        : normalize(curr_delta);
    vec2 dir_prev = dot(prev_delta, prev_delta) < 1e-6 ? dir_curr : normalize(prev_delta);
    vec2 dir_next = dot(next_delta, next_delta) < 1e-6 ? dir_curr : normalize(next_delta);

    vec2 unused_in_plus;
    vec2 unused_in_minus;
    vec2 start_plus;
    vec2 start_minus;
    vec2 unused_start_v1;
    vec2 unused_start_v2;
    float unused_start_turn;
    calculate_join(
        dir_prev, dir_curr,
        unused_in_plus, unused_in_minus, start_plus, start_minus,
        unused_start_v1, unused_start_v2, unused_start_turn);

    vec2 end_plus;
    vec2 end_minus;
    vec2 unused_out_plus;
    vec2 unused_out_minus;
    vec2 v1;
    vec2 v2;
    float turn;
    calculate_join(
        dir_curr, dir_next,
        end_plus, end_minus, unused_out_plus, unused_out_minus,
        v1, v2, turn);

    vec2 clip_scale = 2.0 / u_viewport_size;

    // One core polygon, clipped at both joins. No full rectangles overlap it.
    emit_quad(
        p0.xy + start_plus * clip_scale, p1.xy + end_plus * clip_scale,
        p0.xy + start_minus * clip_scale, p1.xy + end_minus * clip_scale,
        te_color[0], te_color[1]);

    vec2 n_curr = rotate_ccw90(dir_curr) * (u_thickness * 0.5);
    vec2 n_next = rotate_ccw90(dir_next) * (u_thickness * 0.5);
    vec2 curr_corner = turn < 0.0 ? n_curr : -n_curr;
    vec2 next_corner = turn < 0.0 ? n_next : -n_next;
    emit_triangle(
        p1.xy + v1 * clip_scale,
        p1.xy + v2 * clip_scale,
        p1.xy + curr_corner * clip_scale,
        te_color[1]);
    // E1 and E2 are on opposite sides of V1-V2, so reverse the shared edge to
    // retain consistent winding for the second triangle.
    emit_triangle(
        p1.xy + v2 * clip_scale,
        p1.xy + v1 * clip_scale,
        p1.xy + next_corner * clip_scale,
        te_color[1]);
}
