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
layout(location = 1) out vec2 fragTexCoord;


void main() {
    fragTexCoord = inTexCoord;
    gl_PointSize = 2;
    vec4 worldPos = push.modelMatrix * vec4(inPosition, 1.0);
    vec3 worldNormal = mat3(transpose(inverse(push.modelMatrix))) * normal;
    vec3 viewDir = normalize(-vec3(ubo.view * worldPos));

    // Calculate diffuse reflection
    float diffuseStrength = max(dot(worldNormal, -ubo.lightDirection), 0.0);
    vec3 diffuse = diffuseStrength * ubo.lightColor * inColor;

    // Calculate ambient reflection
    vec3 ambient = ubo.ambient * inColor;

    // Calculate final color by combining ambient and diffuse components
    fragColor = ambient + diffuse;

    gl_Position = ubo.proj * ubo.view * worldPos;
}
