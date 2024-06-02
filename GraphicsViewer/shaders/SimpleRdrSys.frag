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


layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inViewVec;
layout(location = 3) in vec3 FragPos;


layout(location = 0) out vec4 outColor;


void main()
{
    vec3 norm = normalize(inNormal);
    vec3 viewDir = normalize(ubo.eyePosition - FragPos);

    // Base ambient lighting
    vec3 ambient = ubo.ambient * ubo.lightColor * inColor;

    // Initialize lighting components
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    // Iterate over each point light
    for (int i = 0; i < ubo.lightCount ; ++i) {
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
    vec3 result = ambient * inColor + diffuse * inColor + specular;
    outColor = vec4(result, 1.0);
}

