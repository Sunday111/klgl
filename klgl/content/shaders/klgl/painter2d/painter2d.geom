// Input from the vertex shader (one point)
layout(points) in;

// Output to the fragment shader (a quad made of 2 triangles)
layout(triangle_strip, max_vertices = 16) out;

// Uniform to control the size of the quad

uniform mat3 u_view;

in mat3 vs_transform[1];
in vec4 vs_color[1];
in uint vs_type[1];

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

void EmitRectLines(float line_width) {
    float half_width = line_width * 0.5f;
    EmitRectangle(vec2(-1 - half_width), vec2(-1 + half_width, 1 + half_width)); // left
    EmitRectangle(vec2(1 - half_width, -1 - half_width), vec2(1 + half_width)); // right
    EmitRectangle(vec2(-1 + half_width, 1 - half_width), vec2(1 - half_width, 1 + half_width)); // top
    EmitRectangle(vec2(-1 + half_width, -1 - half_width), vec2(1 - half_width, -1 + half_width)); // bottom
}


void EmitTriangleLines(float line_width) {
    // Not texture coordinates for now.
    // In general it would require to do this process second time for texture coords and doesn't make a lot of sense for lines
    float half_width = line_width * 0.5f;

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

    float a_dot_c = dot(a, c);
    float j = lb * half_width / sqrt(1 - a_dot_c * a_dot_c);

    // inner directions
    vec2 a_dir = j * (a - c) / lb;
    vec2 b_dir = j * (b - a) / lc;
    vec2 c_dir = j * (c - b) / la;

    vec2 a_inner = va + a_dir;
    vec2 b_inner = vb + b_dir;
    vec2 c_inner = vc + c_dir;
    vec2 a_outer = va - a_dir;
    vec2 b_outer = vb - b_dir;
    vec2 c_outer = vc - c_dir;

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
    gs_color = vs_color[0];
    gs_type = vs_type[0];

    switch(vs_type[0])
    {
    case 2u:
        EmitTriangle();
        break;

    case 3u:
        EmitRectLines(0.01f);
        break;

    case 4u:
        EmitTriangleLines(0.01f);
        break;

    default:
        EmitRectangle(vec2(-1, -1), vec2(1, 1));
        break;
    }

}
