// Input from the vertex shader (one point)
layout(points) in;

// Output to the fragment shader (a quad made of 2 triangles)
layout(triangle_strip, max_vertices = 16) out;

// Uniform to control the size of the quad

uniform mat3 u_view;

in mat3 vs_transform[1];
in vec4 vs_color[1];
in uint vs_type[1];
in vec2 vs_params[1];

flat out vec4 gs_color;
flat out uint gs_type;
out vec2 gs_tex_coord;

void EmitOne_World(vec2 pos) {
    gl_Position = vec4(u_view * vec3(pos, 1), 1);
    EmitVertex();
}

void EmitOne(vec2 pos) {
    gl_Position = vec4(u_view * vs_transform[0] * vec3(pos, 1), 1.0);
    gs_tex_coord = (pos + 1) / 2;
    EmitVertex();
}

void EmitTriangle() {
    EmitOne(vec2(-1, -1)); // Bottom-left
    EmitOne(vec2(1, -1)); // Bottom-right
    EmitOne(vec2(-1, 1)); // Top-left
    EndPrimitive();
}

void EmitRectangle(vec2 bot_left, vec2 top_right) {
    vec2 bot_right = vec2(top_right.x, bot_left.y);
    vec2 top_left = vec2(bot_left.x, top_right.y);
    EmitOne(bot_left);
    EmitOne(bot_right);
    EmitOne(top_left);
    EmitOne(top_right);
    EndPrimitive();
}

float sqr(float a) { return a * a; }

void EmitRectLines(float inner_line_width, float outer_line_width) {
    // https://www.desmos.com/geometry/ftp5ozpwsp

    vec2 va = (vs_transform[0] * vec3(-1, -1, 1)).xy;
    vec2 vb = (vs_transform[0] * vec3(-1,  1, 1)).xy;
    vec2 vc = (vs_transform[0] * vec3( 1,  1, 1)).xy;
    vec2 vd = (vs_transform[0] * vec3( 1, -1, 1)).xy;

    vec2 sa = vb - va;
    vec2 sb = vc - vb;
    vec2 sc = vd - vc;
    vec2 sd = va - vd;

    vec2 a = normalize(sa);
    vec2 b = normalize(sb);
    vec2 c = normalize(sc);
    vec2 d = normalize(sd);

    vec2 a_dir = (a - d) / sqrt(1 - sqr(dot(a, d)));
    vec2 b_dir = (b - a) / sqrt(1 - sqr(dot(b, a)));
    vec2 c_dir = (c - b) / sqrt(1 - sqr(dot(c, b)));
    vec2 d_dir = (d - c) / sqrt(1 - sqr(dot(d, c)));

    vec2 a_inner = va + inner_line_width * a_dir;
    vec2 a_outer = va - outer_line_width * a_dir;

    vec2 b_inner = vb + inner_line_width * b_dir;
    vec2 b_outer = vb - outer_line_width * b_dir;

    vec2 c_inner = vc + inner_line_width * c_dir;
    vec2 c_outer = vc - outer_line_width * c_dir;
    
    vec2 d_inner = vd + inner_line_width * d_dir;
    vec2 d_outer = vd - outer_line_width * d_dir;

    EmitOne_World(a_outer);
    EmitOne_World(a_inner);
    EmitOne_World(b_outer);
    EmitOne_World(b_inner);
    EndPrimitive();

    EmitOne_World(b_outer);
    EmitOne_World(b_inner);
    EmitOne_World(c_outer);
    EmitOne_World(c_inner);
    EndPrimitive();

    EmitOne_World(c_outer);
    EmitOne_World(c_inner);
    EmitOne_World(d_outer);
    EmitOne_World(d_inner);
    EndPrimitive();

    EmitOne_World(d_outer);
    EmitOne_World(d_inner);
    EmitOne_World(a_outer);
    EmitOne_World(a_inner);
    EndPrimitive();
}


void EmitTriangleLines(float inner_line_width, float outer_line_width) {
    // Not texture coordinates for now.
    // In general it would require to do this process second time for texture coords and doesn't make a lot of sense for lines
    // https://www.desmos.com/geometry/woh1quayud

    vec2 va = (vs_transform[0] * vec3(-1, -1, 1)).xy;
    vec2 vb = (vs_transform[0] * vec3( 1, -1, 1)).xy;
    vec2 vc = (vs_transform[0] * vec3(-1,  1, 1)).xy;

    vec2 sa = vb - va;
    vec2 sb = vc - vb;
    vec2 sc = va - vc;

    float la = length(sa);
    float lb = length(sb);
    float lc = length(sc);

    vec2 a = sa / la;
    vec2 b = sb / lb;
    vec2 c = sc / lc;

    float j = lb / sqrt(1 - sqr(dot(a, c)));

    // inner directions
    vec2 a_dir = j * (a - c) / lb;
    vec2 b_dir = j * (b - a) / lc;
    vec2 c_dir = j * (c - b) / la;

    vec2 a_inner = va + inner_line_width * a_dir;
    vec2 b_inner = vb + inner_line_width * b_dir;
    vec2 c_inner = vc + inner_line_width * c_dir;
    vec2 a_outer = va - outer_line_width * a_dir;
    vec2 b_outer = vb - outer_line_width * b_dir;
    vec2 c_outer = vc - outer_line_width * c_dir;

    EmitOne_World(a_outer);
    EmitOne_World(b_outer);
    EmitOne_World(a_inner);
    EmitOne_World(b_inner);
    EndPrimitive();

    EmitOne_World(b_outer);
    EmitOne_World(c_outer);
    EmitOne_World(b_inner);
    EmitOne_World(c_inner);
    EndPrimitive();

    EmitOne_World(c_outer);
    EmitOne_World(a_outer);
    EmitOne_World(c_inner);
    EmitOne_World(a_inner);
    EndPrimitive();
}

void main()
{
    vec2 p = vs_params[0];

    gs_color = vs_color[0];
    gs_type = vs_type[0];

    switch(vs_type[0])
    {
    case 2u:
        EmitTriangle();
        break;

    case 3u:
        EmitRectLines(p.x, p.y);
        break;

    case 4u:
        EmitTriangleLines(p.x, p.y);
        break;

    default:
        EmitRectangle(vec2(-1, -1), vec2(1, 1));
        break;
    }

}
