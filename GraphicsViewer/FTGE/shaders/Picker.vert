#version 450

struct PointLight {
    vec3    position;
    vec3    color;
    vec3    attuniation;
    float   intensity;
    float   radius;
    float   angle; // (0.0 to 180.0)
    float   exponent;
};

// uniform for camera infor and lights
layout(binding = 0) uniform UniformBufferOject {
    mat4 view;
    mat4 proj;
    uint            lightCount;
    PointLight      lights[100];
} ubo;

// push constanct for general lighting info
layout(push_constant) uniform Push {
    vec3    lightColor;
    vec3    lightDirection;
    float   ambient;
} push;

// vertex data
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 inTexCoord;

// per instance data
layout(location = 4) in mat4 modelMatrix;
layout(location = 8) in mat4 normalMatrix;
layout(location = 12) in vec3 inColor2;
//layout(location = 13) in vec3 modelID;
layout(location = 13) in uint modelID;

// output
layout(location = 0) out uint fragColor;

void main() {
//    fragColor = modelID;
    fragColor = modelID;
    gl_PointSize = 2;
    gl_Position = ubo.proj * ubo.view * modelMatrix * vec4(inPosition, 1.0);
}


