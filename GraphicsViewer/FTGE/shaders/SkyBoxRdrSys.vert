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
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 tangent;


// output
layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;

void main2() {
//    outNormal = inNormal;
//    outColor = inColor;
//    outUV = inTexCoord;
//    gl_Position = ubo.proj * ubo.view * push.modelMatrix * vec4(inPosition, 1.0);
//
//    vec4 pos = ubo.view * vec4(inPosition, 1.0);
//    outNormal = mat3(ubo.view) * inNormal;
//    vec3 lPos = mat3(ubo.view) * ubo.lightDirection.xyz;
//    outLightVec = ubo.lightDirection.xyz - pos.xyz;
//    outViewVec = ubo.eyePosition.xyz - pos.xyz;
}


void main() 
{
	outUV = inPosition;
	// Convert cubemap coordinates into Vulkan coordinate space
	outUV.xy *= -1.0;
	gl_Position = ubo.proj * ubo.view * push.modelMatrix * vec4(inPosition.xyz, 1.0);
}
