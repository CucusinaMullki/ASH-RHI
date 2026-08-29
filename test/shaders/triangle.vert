#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 rotation;
} ubo;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

vec2 uvs[3] = vec2[](
    vec2(0.5, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

void main() {
    gl_Position = ubo.rotation * vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
    fragUV = uvs[gl_VertexIndex];
}