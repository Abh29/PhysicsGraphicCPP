#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;

layout(binding = 1) uniform samplerCube cubeSampler;

layout(location = 0) out vec4 outColor;

void main() 
{
	outColor = texture(cubeSampler, inUV);
}
