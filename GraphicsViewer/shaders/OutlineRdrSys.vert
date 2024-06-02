#version 450
// uniform for camera infor and lights
layout(binding = 0) uniform UniformBufferOject {
    mat4            view;
    mat4            proj;
    vec3            eyePosition;
} ubo;

layout(push_constant) uniform Push {
    mat4            model;
    vec3            baseColor;
    uint            modelID;
} push;

// vertex data
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 tangent;


void main() {
    // fragColor = vec3(0.945, 0.394, 0.027);
    vec4 pos = vec4(inPosition.xyz + inNormal * 0.22, 1.0);
    gl_Position = ubo.proj * ubo.view * push.model * pos;
}

