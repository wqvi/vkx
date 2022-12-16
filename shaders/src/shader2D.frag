#version 450

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D materialDiffuse;

layout(binding = 2) uniform Light {
    vec3 position;
    vec3 eyePosition;
    vec4 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float constant;
    float linear;
    float quadratic;
} light;

layout(binding = 3) uniform Material {
    vec3 specularColor;
    float shininess;
} material;

layout(location = 0) in Data {
	vec2 worldPos;
	vec2 uv;
	vec2 worldNormal;
} data;

void main() {
    outColor = vec4(1.0);
}