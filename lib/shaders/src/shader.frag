#version 450

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D materialDiffuse;

layout(binding = 2) uniform Light 
{
    vec3 position;
    vec3 eyePosition;
    vec4 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float constant;
    float linear;
    float quadratic;
} light;

layout(binding = 3) uniform Material
{
    vec3 specularColor;
    float shininess;
} material;

layout(location = 0) in Data
{
	vec3 worldPos;
	vec2 uv;
	vec3 worldNormal;
} data;

void main()
{
    vec3 ambient = vec3(light.ambientColor.rgb * light.ambientColor.w) * texture(materialDiffuse, data.uv).rgb;

    vec3 lightDir = normalize(light.position - data.worldPos);
    float diff = max(dot(data.worldNormal, lightDir), 0.0);
    vec3 diffuse = light.diffuseColor * diff * texture(materialDiffuse, data.uv).rgb;  
    
    vec3 viewDir = normalize(light.eyePosition - data.worldPos);
    vec3 reflectDir = reflect(-lightDir, data.worldNormal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specularColor * spec * material.specularColor;  

    vec3 directionToLight = light.position - data.worldPos;
    float attenuation = 1.0 / dot(directionToLight, directionToLight);

    float lightDistance = length(light.position - data.worldPos);
    attenuation = 1.0 / (light.constant + light.linear * lightDistance + light.quadratic * (lightDistance * lightDistance)); 

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;
    outColor = vec4(result, 1.0);
}