#version 450

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;


// uniform for camera infor and lights
layout(binding = 0) uniform UniformBufferOject {
    vec3            lightColor;
    vec3            lightDirection;
    float           ambient;
    mat4            view;
    mat4            proj;
    vec3            eyePosition;
    uint            lightCount;
} ubo;


layout(push_constant) uniform Push {
    mat4            model;
    vec3            baseColor;
    uint            modelID;
} push;



layout (location = 0) in vec3 inNormal[];

layout (location = 0) out vec3 outColor;

void main(void)
{	
	float normalLength = 0.2;
	for(int i=0; i<gl_in.length(); i++)
	{
		vec3 pos = gl_in[i].gl_Position.xyz;
		vec3 normal = inNormal[i].xyz;

		gl_Position = ubo.proj * ubo.view * push.model * vec4(pos, 1.0);
		outColor = vec3(1.0, 0.0, 0.0);
		EmitVertex();

		gl_Position = ubo.proj * ubo.view * push.model * vec4(pos + normal * normalLength, 1.0);
		outColor = vec3(0.0, 0.0, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}
