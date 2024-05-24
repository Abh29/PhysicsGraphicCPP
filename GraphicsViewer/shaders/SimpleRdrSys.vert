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
    vec3            lightColor;
    vec3            lightDirection;
    float           ambient;
    mat4            view;
    mat4            proj;
    vec3            eyePosition;
    uint            lightCount;
    PointLight      lights[100];
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
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outLightVec;
layout(location = 3) out vec3 outViewVec;
layout(location = 4) out vec3 outLightColor;
layout(location = 5) out vec3 FragPos;
layout(location = 6) out float ambient;


void main()
{
    gl_PointSize = 2;
    FragPos = vec3(push.modelMatrix * vec4(inPosition, 1.0));
    outNormal = mat3(transpose(inverse(push.modelMatrix))) * normal; 
    gl_Position = ubo.proj * ubo.view * push.modelMatrix * vec4(inPosition, 1.0);
    outLightVec = ubo.lightDirection.xyz;
    outLightColor = ubo.lightColor;
    fragColor = inColor * push.baseColor;
    ambient = ubo.ambient;
}

void simple_unshaded() {
    gl_PointSize = 2;
    gl_Position = ubo.proj * ubo.view * push.modelMatrix * vec4(inPosition, 1.0f);
    fragColor = inColor;
}

