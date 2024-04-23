#version 450

//layout(location = 0) in vec3 fragColor;
//layout(location = 0) out vec4 outColor;
layout(location = 0) flat in uint fragid;
layout(location = 0) out uint outColor;

void main() {
//    outColor = vec4(fragColor, 1.0f);
    outColor = fragid;
}
