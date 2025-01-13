#version 330 core

in vec4 fragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main() {
	float d = length(fragPos.xyz - lightPos);
	d = d / farPlane; // assume near plane is at 0
	gl_FragDepth = d;
}