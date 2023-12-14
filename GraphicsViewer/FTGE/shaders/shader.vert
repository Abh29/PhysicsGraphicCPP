#version 450

struct Camera {
    mat4 view;
    mat4 proj;
};

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
    uint            lightCount;
    Camera          camera;
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
layout(location = 8) in vec3 inColor2;

// output
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
//    gl_Position = vec4(inPosition, 1.0f);
//    fragColor = vec3(1.0f, 0.0f, 0.0f);

//    gl_Position = ubo.camera.proj * ubo.camera.view * modelMatrix * vec4(inPosition, 1.0);

//    vec3 n = normalize(normal);
//    vec3 amb = push.ambient * inColor2;
//    float diffuseStrength = max(dot(n, -push.lightDirection), 0.0);
//    vec3 diffuse = diffuseStrength * push.lightColor * inColor2;
//    fragColor = amb + diffuse;

//    fragColor = inColor2 * (normalize(push.lightDirection) * normal);
    fragColor = inColor2;
    fragTexCoord = inTexCoord;
    gl_PointSize = 2;

    // Transform the vertex position, normal, and calculate the light direction in view space
    vec4 worldPos = modelMatrix * vec4(inPosition, 1.0);
    vec3 worldNormal = mat3(transpose(inverse(modelMatrix))) * normal;
    vec3 viewDir = normalize(-vec3(ubo.camera.view * worldPos));

    // Calculate diffuse reflection
    float diffuseStrength = max(dot(worldNormal, -push.lightDirection), 0.0);
    vec3 diffuse = diffuseStrength * push.lightColor * inColor2;

    // Calculate ambient reflection
    vec3 ambient = push.ambient * inColor2;
    // Calculate final color by combining ambient and diffuse components
    fragColor = ambient + diffuse;
    // Transform the vertex position to clip space for later use in the fragment shader
    gl_Position = ubo.camera.proj * ubo.camera.view * worldPos;
}
