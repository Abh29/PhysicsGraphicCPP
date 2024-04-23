#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;


void main() {
    vec4 color = texture(texSampler, inUV) * vec4(inColor, 1.0);

    vec3 N = normalize(inNormal);
    vec3 L = normalize(inLightVec);
//    vec3 V = normalize(inViewVec);
//    vec3 R = reflect(L, N);
    vec3 diffuse = max(dot(N, L), 0.15) * inColor;
//    vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
//    outColor = vec4(diffuse * color.rgb + specular, 1.0);
    outColor = vec4(diffuse * color.rgb, 1.0);
}

void main2() {
//      outColor = texture(texSampler, fragTexCoord) * vec4(fragColor, 1.0);
//    outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0f);
//    outColor = texture(texSampler, fragTexCoord);
}
