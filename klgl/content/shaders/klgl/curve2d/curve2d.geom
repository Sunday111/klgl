layout(lines) in;
layout(triangle_strip,max_vertices=4) out;

in vec4 te_color[];
in vec2 te_prev_point[];
in vec2 te_next_point[];
out vec4 gs_color;
uniform float u_thickness;
uniform vec2 u_viewport_size;

vec2 rotate_ccw90(vec2 v) {
    return vec2(-v.y, v.x);
}

void main(){
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec2 sp = (te_prev_point[0].xy + 1) * u_viewport_size / 2;
    vec2 s0 = (p0.xy + 1) * u_viewport_size / 2;
    vec2 s1 = (p1.xy + 1) * u_viewport_size / 2;
    vec2 sn = (te_next_point[0].xy + 1) * u_viewport_size / 2;
    float pl = length(s0 - sp);
    float nl = length(sn - s1);
    vec2 dir_curr = normalize(s1-s0);
    vec2 dir_prev = pl < 0.5 ? dir_curr : (s0 - sp) / pl;
    vec2 dir_next = nl < 0.5 ? dir_curr : (sn - s1) / nl;
    vec2 n_curr = u_thickness * rotate_ccw90(dir_curr) / 2;
    vec2 off_curr = n_curr/u_viewport_size*2.0;
    vec4 v0=vec4(p0.xy+off_curr,0,1);
    vec4 v1=vec4(p1.xy+off_curr,0,1);
    vec4 v2=vec4(p0.xy-off_curr,0,1);
    vec4 v3=vec4(p1.xy-off_curr,0,1);
    gs_color = te_color[0]; gl_Position=v0; EmitVertex();
    gs_color = te_color[1]; gl_Position=v1; EmitVertex();
    gs_color = te_color[0]; gl_Position=v2; EmitVertex();
    gs_color = te_color[1]; gl_Position=v3; EmitVertex();
    EndPrimitive();
}
