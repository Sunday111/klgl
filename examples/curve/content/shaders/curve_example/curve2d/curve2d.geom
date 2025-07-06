layout(lines) in;
layout(triangle_strip,max_vertices=4) out;

in vec4 te_color[];
out vec4 gs_color;
uniform float u_thickness;
uniform vec2 u_viewport_size;
void main(){
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec2 ndc0=p0.xy/p0.w, ndc1=p1.xy/p1.w;
    vec2 s0=(ndc0*0.5+0.5)*u_viewport_size;
    vec2 s1=(ndc1*0.5+0.5)*u_viewport_size;
    vec2 d=normalize(s1-s0);
    vec2 n=vec2(-d.y,d.x)*0.5*u_thickness;
    vec2 off = n/u_viewport_size*2.0;
    vec4 v0=vec4(ndc0+off,0,1);
    vec4 v1=vec4(ndc1+off,0,1);
    vec4 v2=vec4(ndc0-off,0,1);
    vec4 v3=vec4(ndc1-off,0,1);
    gs_color = te_color[0]; gl_Position=v0; EmitVertex();
    gs_color = te_color[1]; gl_Position=v1; EmitVertex();
    gs_color = te_color[0]; gl_Position=v2; EmitVertex();
    gs_color = te_color[1]; gl_Position=v3; EmitVertex();
    EndPrimitive();
}
