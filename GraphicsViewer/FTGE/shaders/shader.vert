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


void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}

void main2() {
//    gl_Position = vec4(inPosition, 1.0f);
//    fragColor = vec3(1.0f, 0.0f, 0.0f);

//    gl_Position = ubo.camera.proj * ubo.camera.view * modelMatrix * vec4(inPosition, 1.0);

//    vec3 n = normalize(normal);
//    vec3 amb = push.ambient * inColor2;
//    float diffuseStrength = max(dot(n, -push.lightDirection), 0.0);
//    vec3 diffuse = diffuseStrength * push.lightColor * inColor2;
//    fragColor = amb + diffuse;

//    fragColor = inColor2 * (normalize(push.lightDirection) * normal);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    gl_PointSize = 2;

//     Transform the vertex position, normal, and calculate the light direction in view space
    vec4 worldPos = push.modelMatrix * vec4(inPosition, 1.0);
    vec3 worldNormal = mat3(transpose(inverse(push.modelMatrix))) * normal;
//    vec3 worldNormal = mat3(normalMatrix) * normal;
    vec3 viewDir = normalize(-vec3(ubo.view * worldPos));

    // Calculate diffuse reflection
    float diffuseStrength = max(dot(worldNormal, -ubo.lightDirection), 0.0);
    vec3 diffuse = diffuseStrength * ubo.lightColor * inColor;

    // Calculate ambient reflection
    vec3 ambient = ubo.ambient * inColor;
    // Calculate final color by combining ambient and diffuse components
    fragColor = ambient + diffuse;
//    fragColor = inColor2;
//     Transform the vertex position to clip space for later use in the fragment shader
    gl_Position = ubo.proj * ubo.view * worldPos;
//    gl_Position = ubo.proj * ubo.view * modelMatrix * vec4(inPosition, 1.0);
}

