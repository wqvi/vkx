#version 450

/* Vertex Data */
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

/* Data to be passed into the fragment shader */
layout (location = 0) out Data
{
    vec3 worldPos;
    vec2 uv;
    vec3 worldNormal;
} data;

/* 
 * Commonly known as the MVP. Not to be confused with most valuable player!
 * The model, view, and projection matrices uniform buffer object.
 * The equation is: gl_Position = (projection * view * model) * vec4(pos, 1.0);
 */
layout (binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    data.worldPos = vec3(ubo.model * vec4(aPos, 1.0));
    data.uv = aUV;
    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
    data.worldNormal = normalize(normalMatrix * aNormal);
    
    gl_Position = ubo.proj * ubo.view * vec4(data.worldPos, 1.0);
}