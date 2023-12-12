#version 450

struct UniformBufferOject {
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(binding = 0) uniform ubos {
    UniformBufferOject  objects[100];
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
    mat4 view;
    mat4 proj;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 3) in mat4 modelMatrix;
layout(location = 7) in vec3 inColor2;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
//    gl_Position = vec4(inPosition, 1.0f);
//    fragColor = vec3(1.0f, 0.0f, 0.0f);
    gl_Position = ubo.objects[gl_InstanceIndex].proj * push.view * ubo.objects[gl_InstanceIndex].model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    gl_PointSize = 2;
}
