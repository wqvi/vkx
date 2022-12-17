#version 450

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec2 aNormal;

layout (location = 0) out Data {
    vec2 worldPos;
    vec2 uv;
    vec2 worldNormal;
} data;

layout (binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

void main() {    
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(aPos, 1.0, 1.0);
}