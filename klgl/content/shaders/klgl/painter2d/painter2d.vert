in mat3 a_transform;
in uint a_type;
in vec4 a_color;

out mat3 vs_transform;
out vec4 vs_color;
out uint vs_type;


void main()
{
    vs_transform = a_transform;
    vs_color = a_color;
    vs_type = a_type;
}
