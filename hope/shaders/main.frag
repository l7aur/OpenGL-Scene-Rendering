#version 330 core

in vec4 vCol;
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

out vec4 colour;

const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 3;

struct Light {
	vec3 colour;
	float ambientIntensity;
	float diffuseIntensity;
};

struct DirectionalLight {
	Light base;
	vec3 direction;
};

struct PointLight {
	Light base;
	vec3 position;
	float constant, linear, exponent;
};

struct SpotLight {
	PointLight base;
	vec3 direction;
	float edge;
};

struct Material {
	float specularIntensity;
	float shininess;
};

uniform sampler2D theTexture;

uniform DirectionalLight directionalLight;

uniform int pointLightCount;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

uniform int spotLightCount;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform Material material;
uniform vec3 eyePosition;

vec4 computeLightByDirection(Light light, vec3 direction) {
	vec4 ambientColour = vec4(light.colour, 1.0f) * light.ambientIntensity;

	float diffuseFactor = max(dot(normalize(normal), normalize(direction)), 0.0f);
	vec4 diffuseColour = vec4(light.colour * light.diffuseIntensity * diffuseFactor, 1.0f);

	vec4 specularColour = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	
	if(diffuseFactor > 0.0f) {
		vec3 fragToEye = normalize(eyePosition - fragPos);
		vec3 reflectedVertex = normalize(reflect(direction, normalize(normal)));

		float specularFactor = dot(fragToEye, reflectedVertex);
		if(specularFactor > 0.0f) {
			specularFactor = pow(specularFactor, material.shininess);
			specularColour = vec4(light.colour * material.specularIntensity * specularFactor, 1.0f);
		}
	}
	return ambientColour + diffuseColour + specularColour;
}

vec4 computeOnePointLight(PointLight p) {
	vec3 direction = fragPos - p.position;
	float d = length(direction);
	direction = normalize(direction);

	vec4 colour = computeLightByDirection(p.base, direction);
	float attentuation = p.exponent * d * d + p.linear * d + p.constant;
	return colour / attentuation;
}

vec4 computeOneSpotLight(SpotLight s) {
	vec3 rayDirection = normalize(fragPos - s.base.position);
	float slFactor = dot(rayDirection, s.direction);
	if(slFactor > s.edge) {
		vec4 colour = computeOnePointLight(s.base);
		return colour * (1.0f - (1.0f - slFactor) * (1.0f / (1.0f - s.edge))); //map to 0 1
	}
	return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

vec4 computePointLights() {
	vec4 totalColour = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for(int i = 0; i < pointLightCount; i++)
		totalColour += computeOnePointLight(pointLights[i]);
	return totalColour;
}

vec4 computeSpotLights() {
	vec4 totalColour = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for(int i = 0; i < spotLightCount; i++)
		totalColour += computeOneSpotLight(spotLights[i]);
	return totalColour;
}

vec4 computeDirectionalLight() {
	return computeLightByDirection(directionalLight.base, directionalLight.direction);
}

void main() {
	vec4 finalColour = computeDirectionalLight() + computePointLights() + computeSpotLights();
	colour = texture(theTexture, texCoord) * finalColour;
}