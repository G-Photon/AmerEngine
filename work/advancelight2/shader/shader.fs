#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;    
    sampler2D emission;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    vec3 lightDir = normalize(light.position - FragPos);
    //vec3 lightDir = normalize(-light.direction);
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);    

    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb * attenuation;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb * attenuation;  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfDir, norm), 0.0), material.shininess*4.0);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb * attenuation;  
    
      // emission
    vec3 emission = texture(material.emission, TexCoords).rgb;
        
    vec3 result = ambient + diffuse * intensity + specular * intensity;
    FragColor = vec4(result, 1.0);
} 