in vec3 a_position;
in vec3 a_normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 vs_normal;
out vec3 vs_frag_pos;

void main()
{
    gl_Position = u_projection * (u_view * (u_model * vec4(a_position, 1)));
    vs_frag_pos = vec3(u_model * vec4(a_position, 1));
    vs_normal = mat3(transpose(inverse(u_model))) * a_normal;
}
