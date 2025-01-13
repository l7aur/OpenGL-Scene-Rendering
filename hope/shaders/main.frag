#version 330 core

in vec4 vCol;
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;
in vec4 directionalLightSpacePos;

out vec4 colour;

const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 3;

vec3 sampleOffsetDirections[20] = vec3[] (
	vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
	vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
	vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, 1), vec3(0, 1, -1)
);

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

struct OmniShadowMap {
	samplerCube shadowMap;
	float farPlane;
};

struct Material {
	float specularIntensity;
	float shininess;
};

uniform sampler2D theTexture;
uniform sampler2D directionalShadowMap;
uniform OmniShadowMap omniShadowMaps[MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS];

uniform DirectionalLight directionalLight;

uniform int pointLightCount;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

uniform int spotLightCount;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform Material material;
uniform vec3 eyePosition;

float computeOmniShadowFactor(PointLight light, int shadowIndex) {
	vec3 fragToLight = fragPos - light.position;

	float currentDepth = length(fragToLight);
	
	float shadow = 0.0f;
    float bias = 0.05f;
//	float samples = 4.0f;
//	float offset = 0.1f;
//
//	for(float x = -offset; x < offset; x += offset / (samples * 0.5f)) 
//		for(float y = -offset; y < offset; y += offset / (samples * 0.5f)) 
//			for(float z = -offset; z < offset; z += offset / (samples * 0.5f)) {
//				float closestDepth = texture(omniShadowMaps[shadowIndex].shadowMap, fragToLight + vec3(x, y, z)).r;
//				closestDepth *= omniShadowMaps[shadowIndex].farPlane;
//				if(currentDepth - bias > closestDepth)
//					shadow += 1.0f;
//			}
//
//	shadow /= (samples * samples * samples);
//	return shadow;
	int samples = 20;

	float viewDistance = length(eyePosition - fragPos);
	float diskRadius = (1.0f + (viewDistance / omniShadowMaps[shadowIndex].farPlane)) / 25.0f;
	
	
	for(int i = 0; i < samples; i++) {
		float closestDepth = texture(omniShadowMaps[shadowIndex].shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= omniShadowMaps[shadowIndex].farPlane;
		if(currentDepth - bias > closestDepth)
			shadow += 1.0f;
	}

	shadow /= float(samples);
	return shadow;
}

float computeDirectionalShadowFactor(DirectionalLight light) {
	vec3 projCoords = directionalLightSpacePos.xyz / directionalLightSpacePos.w;
	projCoords = (projCoords * 0.5) + 0.5;
	float current = projCoords.z;
	vec3 normal = normalize(normal);
	vec3 lightDir = normalize(directionalLight.direction);
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.0005);
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(directionalShadowMap, 0);
	for(int x = -1; x <= 1; ++x)
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(directionalShadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
			shadow += current - bias > pcfDepth ? 1.0 : 0.0;
		}
	shadow /= 9.0;
	if(projCoords.z > 1.0)
		shadow = 0.0;				
	return shadow;
}

vec4 computeLightByDirection(Light light, vec3 direction, float shadowFactor) {
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
	return (ambientColour + (1.0f - shadowFactor) * (diffuseColour + specularColour));
}

vec4 computeOnePointLight(PointLight p, int shadowIndex) {
	vec3 direction = fragPos - p.position;
	float d = length(direction);
	direction = normalize(direction);

	float shadowFactor = computeOmniShadowFactor(p, shadowIndex);

	vec4 colour = computeLightByDirection(p.base, direction, shadowFactor);
	float attentuation = p.exponent * d * d + p.linear * d + p.constant;
	return colour / attentuation;
}

vec4 computeOneSpotLight(SpotLight s, int shadowIndex) {
	vec3 rayDirection = normalize(fragPos - s.base.position);
	float slFactor = dot(rayDirection, s.direction);
	if(slFactor > s.edge) {
		vec4 colour = computeOnePointLight(s.base, shadowIndex);
		return colour * (1.0f - (1.0f - slFactor) * (1.0f / (1.0f - s.edge))); //map to 0 1
	}
	return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

vec4 computePointLights() {
	vec4 totalColour = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for(int i = 0; i < pointLightCount; i++)
		totalColour += computeOnePointLight(pointLights[i], i);
	return totalColour;
}

vec4 computeSpotLights() {
	vec4 totalColour = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for(int i = 0; i < spotLightCount; i++)
		totalColour += computeOneSpotLight(spotLights[i], i + pointLightCount);
	return totalColour;
}

vec4 computeDirectionalLight() {
	float shadowF = computeDirectionalShadowFactor(directionalLight);
 	return computeLightByDirection(directionalLight.base, directionalLight.direction, shadowF);
}

void main() {
	vec4 finalColour = computeDirectionalLight() + computePointLights() + computeSpotLights();
	colour = texture(theTexture, texCoord) * finalColour;
}