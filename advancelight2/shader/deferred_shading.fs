#version 330 core
out vec4 FragColor;


uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;
    float Linear;
    float Quadratic;
};
Light pointlight;
uniform vec2 screenSize;
uniform vec3 viewPos;

vec2 CalcTexCoord()
{
   return gl_FragCoord.xy / screenSize;
}

void main()
{    
    vec2 TexCoords = CalcTexCoord();         
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    float dis = length(pointlight.Position - FragPos);
    // diffuse
    vec3 lightDir = normalize(pointlight.Position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * pointlight.Color;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    vec3 specular = pointlight.Color * spec * Specular;
    // attenuation
    float attenuation = 1.0 / (1.0 + pointlight.Linear * dis + pointlight.Quadratic * dis * dis);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;
    float gamma=2.2;
    lighting =pow(lighting,vec3(1/gamma));
    FragColor = vec4(lighting, 1.0);
}