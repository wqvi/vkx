#version 450

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D materialDiffuse;

layout (location = 0) in vec2 fragUV;

void main() {
    outColor = texture(materialDiffuse, fragUV);
}