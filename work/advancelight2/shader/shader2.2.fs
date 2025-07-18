#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;
in vec3 LightPos;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 viewPos;
void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(- FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    // combine results
    vec3 result = (ambient + diffuse + specular) * objectColor;

    FragColor = vec4(result, 1.0);
}