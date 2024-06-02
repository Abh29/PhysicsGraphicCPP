#version 450

// uniform for camera infor and lights
layout(binding = 0) uniform UniformBufferOject {
    mat4            view;
    mat4            proj;
    vec3            eyePosition;
} ubo;

// push constanct for general lighting info
layout(push_constant) uniform Push {
    mat4            modelMatrix;
    vec3            baseColor;
    uint            modelID;
} push;

// vertex data
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 tangent;

// output
layout(location = 0) out uint fragColor;


void main() {
    fragColor = push.modelID;
    gl_Position = ubo.proj * ubo.view * push.modelMatrix * vec4(inPosition, 1.0);
}


