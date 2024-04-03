#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inLightVec;
layout(location = 3) in vec3 inViewVec;

layout(location = 0) out vec4 outColor;

void main1() {
    outColor = vec4(inColor, 1.0f);
}



void main2() 
{
	vec3 color;
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	float intensity = dot(N,L);
	if (intensity > 0.98)
		color = inColor * 1.5;
	else if  (intensity > 0.9)
		color = inColor * 1.0;
	else if (intensity > 0.5)
		color = inColor * 0.6;
	else if (intensity > 0.25)
		color = inColor * 0.4;
	else
		color = inColor * 0.2;
	// Desaturate a bit
	color = vec3(mix(color, vec3(dot(vec3(0.2126,0.7152,0.0722), color)), 0.1));	
	outColor.rgb = color;
}

void main() {
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);
	vec3 ambient = vec3(0.1);
	vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0);
	vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
	outColor = vec4((ambient + diffuse) * inColor.rgb + specular, 1.0);		
}

