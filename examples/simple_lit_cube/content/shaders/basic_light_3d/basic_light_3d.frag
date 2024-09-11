uniform float u_ambient;
uniform float u_specular;
uniform vec3 u_light_color;
uniform vec4 u_object_color;
uniform vec3 u_light_pos;
uniform vec3 u_view_pos;

in vec3 vs_normal;
in vec3 vs_frag_pos;

out vec4 FragColor;

void main()
{
    vec3 ambient = u_ambient * u_light_color;

    vec3 norm = normalize(vs_normal);
    vec3 light_dir = normalize(u_light_pos - vs_frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * u_light_color;

    vec3 view_dir = normalize(u_view_pos - vs_frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 64);
    vec3 specular = u_specular * spec * u_light_color;

    FragColor = vec4(ambient + diffuse + specular, 1) * u_object_color;
}
