layout(lines) in;
layout(triangle_strip,max_vertices=6) out;

in vec4 te_color[];
in vec4 te_join_normals[];
in vec4 te_join_vertices[];
out vec4 gs_color;

void emit(vec2 position, vec4 color) {
    gs_color = color;
    gl_Position = vec4(position, 0, 1);
    EmitVertex();
}

void main(){
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec2 start_normal = te_join_normals[0].zw;
    vec2 start_v1 = te_join_vertices[0].xy;
    vec2 start_v2 = te_join_vertices[0].zw;
    float start_turn = te_join_normals[0].x * start_normal.y - te_join_normals[0].y * start_normal.x;
    vec2 start_plus = start_turn < 0.0 ? start_normal : start_v2;
    vec2 start_minus = start_turn < 0.0 ? start_v1 : -start_normal;

    vec2 end_normal = te_join_normals[1].xy;
    vec2 next_normal = te_join_normals[1].zw;
    vec2 v1 = te_join_vertices[1].xy;
    vec2 v2 = te_join_vertices[1].zw;
    float end_turn = end_normal.x * next_normal.y - end_normal.y * next_normal.x;
    vec2 end_plus = end_turn < 0.0 ? end_normal : v2;
    vec2 end_minus = end_turn < 0.0 ? v1 : -end_normal;

    vec2 next_corner = end_turn < 0.0 ? next_normal : -next_normal;

    // The clipped core and both connector triangles form one continuous strip.
    // Reversing its sides for a counter-clockwise turn keeps all four triangle
    // windings consistent while retaining V1-V2 as the connector edge.
    if (end_turn < 0.0) {
        emit(p0.xy + start_plus, te_color[0]);
        emit(p0.xy + start_minus, te_color[0]);
        emit(p1.xy + end_plus, te_color[1]);
        emit(p1.xy + end_minus, te_color[1]);
        emit(p1.xy + v2, te_color[1]);
        emit(p1.xy + next_corner, te_color[1]);
    } else if (end_turn > 0.0) {
        emit(p0.xy + start_minus, te_color[0]);
        emit(p0.xy + start_plus, te_color[0]);
        emit(p1.xy + end_minus, te_color[1]);
        emit(p1.xy + end_plus, te_color[1]);
        emit(p1.xy + v1, te_color[1]);
        emit(p1.xy + next_corner, te_color[1]);
    } else {
        // Both connector triangles have zero area on an exactly straight join.
        emit(p0.xy + start_minus, te_color[0]);
        emit(p0.xy + start_plus, te_color[0]);
        emit(p1.xy + end_minus, te_color[1]);
        emit(p1.xy + end_plus, te_color[1]);
    }
    EndPrimitive();
}
