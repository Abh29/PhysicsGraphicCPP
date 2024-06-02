#version 450

struct PointLight {
    vec3    position;
    vec3    color;
    vec3    attenuation;
    float   ambient;
    float   diffuse;
    float   specular;
    uint    on;
};

// uniform for camera infor and lights
layout(binding = 0) uniform UniformBufferOject {
    mat4            view;
    mat4            proj;
    vec3            eyePosition;
    vec3            lightColor;
    vec3            lightDirection;
    float           ambient;
    float	    pointSize;
    uint            lightCount;
    PointLight      lights[10];
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
layout(location = 2) out vec3 outViewVec;
layout(location = 3) out vec3 FragPos;


void main()
{
    gl_PointSize = ubo.pointSize;
    FragPos = vec3(push.modelMatrix * vec4(inPosition, 1.0));
    outNormal = mat3(transpose(inverse(push.modelMatrix))) * normal; 
    gl_Position = ubo.proj * ubo.view * vec4(FragPos, 1.0);
    fragColor = push.baseColor;
}


