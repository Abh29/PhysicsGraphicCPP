#version 450

// vertex data
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 tangent;


// output
layout(location = 0) out vec3 outNormal;

void main() {
    gl_Position = vec4(inPosition.xyz, 1.0f);
    outNormal = normalize(inNormal);
}



