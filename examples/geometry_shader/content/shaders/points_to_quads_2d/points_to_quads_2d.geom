// Input from the vertex shader (one point)
layout(points) in;

// Output to the fragment shader (a quad made of 2 triangles)
layout(triangle_strip, max_vertices = 4) out;

// Uniform to control the size of the quad

in mat3 vs_transform[1];
in vec4 vs_color[1];
in uint vs_type[1];

out vec4 gs_color;
flat out uint gs_type;
out vec2 gs_tex_coord;

void main()
{
    gs_color = vs_color[0];
    gs_type = vs_type[0];

    // Create the quad by emitting four vertices in a triangle strip
    // Bottom-left
    gl_Position = vec4(vs_transform[0] * vec3(-1, -1, 1), 1.0);
    gs_tex_coord = vec2(0, 0);
    EmitVertex();

    // Bottom-right
    gl_Position = vec4(vs_transform[0] * vec3(1, -1, 1), 1.0);
    gs_tex_coord = vec2(1, 0);
    EmitVertex();

    // Top-left
    gl_Position = vec4(vs_transform[0] * vec3(-1, 1, 1), 1.0);
    gs_tex_coord = vec2(0, 1);
    EmitVertex();

    if (vs_type[0] != 2u)
    {
        // Top-right
        gl_Position = vec4(vs_transform[0] * vec3(1, 1, 1), 1.0);
        gs_tex_coord = vec2(1, 1);
        EmitVertex();
    }

    // End primitive to complete the quad
    EndPrimitive();
}
