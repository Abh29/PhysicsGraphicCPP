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


layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 FragPos;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;


void main()
{
    vec3 norm = normalize(inNormal);
    vec3 viewDir = normalize(ubo.eyePosition - FragPos);


    // Sample the texture
    vec4 texColor = texture(texSampler, inUV);

    // Base ambient lighting
    vec3 ambient = ubo.lightColor *  texColor.rgb;

    // Initialize lighting components
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    // Iterate over each point light
    for (int i = 0; i < ubo.lightCount; ++i) {
        PointLight light = ubo.lights[i];
        if (light.on == 0) continue; // Skip if the light is off

        // Calculate attenuation
        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * distance * distance);

        // Ambient contribution from point light
        ambient += light.ambient * attenuation * light.color;

        // Diffuse contribution
        vec3 lightDir = normalize(light.position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse += light.diffuse * diff * light.color * attenuation;

        // Specular contribution
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Shininess factor
        specular += light.specular * spec * light.color * attenuation;
    }


    // Combine results

    vec3 result = ambient +  diffuse * texColor.rgb + specular;
    outColor = vec4(result, 1.0f);
}


void main1() {
//    vec4 color = texture(texSampler, inUV) * vec4(inColor, 1.0);
//
//    vec3 N = normalize(inNormal);
//    vec3 L = normalize(inLightVec);
//    vec3 V = normalize(inViewVec);
//    vec3 R = reflect(L, N);
//    vec3 diffuse = max(dot(N, L), 0.15) * inColor;
//    vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
//    outColor = vec4(diffuse * color.rgb + specular, 1.0);
 //   outColor = vec4(diffuse * color.rgb, 1.0);
}

void main2() {
//      outColor = texture(texSampler, fragTexCoord) * vec4(fragColor, 1.0);
//    outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0f);
//    outColor = texture(texSampler, fragTexCoord);
}
