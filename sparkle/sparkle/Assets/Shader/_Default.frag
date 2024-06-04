#version 460 core

struct Material {
	sampler2D diffuse;
	bool useTexture;
	vec3 tint;

	float specular;
	float smoothness;
};

struct SpotLight {
	vec3 position;
	vec3 direction;
	float cutoff;
	float outerCutoff;

	float constant;
	float linear;
	float quadratic;

	vec3 color;
	float ambient;
};

in VS {
	vec3 worldPos;
	vec3 normal;
	vec2 uv;
	vec4 lightSpaceFragPos;
} vs;

uniform vec3 _CameraPos;
uniform sampler2D _ShadowTex;
uniform SpotLight spotLight;
uniform Material material;

out vec4 FragColor;

float ShadowCalculation(float ndotl, vec4 lightSpaceFragPos)
{
	// This is standard stuff from learnopengl.com, reference that if not clear.

	// perform perspective divide
	vec3 projCoords = lightSpaceFragPos.xyz / lightSpaceFragPos.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from the light's perspective
	float closestDepth = texture(_ShadowTex, projCoords.xy).r;
	// get depth of the current fragment from light's perspective
	float currentDepth = projCoords.z;
	// Calculate bias based on depth map resolution and slope
	float bias = 0.01 * max(1.0 - ndotl, 0.1);
	// Percentage-closer filtering (PCF) to smooth shadow edges
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(_ShadowTex, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for(int y= -1; y <= 1; ++y)
		{
			float pcfDepth = texture(_ShadowTex, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	// Keep the shadow at 0.0 when outside the far_plane region of the light's frustrum.
	if(projCoords.z > 1.0)
	{
		shadow = 0.0;
	}

	return shadow;
}

vec3 CalcSpotLight(SpotLight light, vec3 cameraPos, vec3 normal, vec3 worldPos, vec4 lightSpaceFragPos, Material material, vec3 albedo)
{
	// Spotlight intensity
	vec3 lightDir = normalize(light.position - worldPos);
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutoff - light.outerCutoff;
	float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
	float attenuation = intensity;
	// ambient
	float ambient = light.ambient * max(0.3, attenuation);
	// diffuse
	float ndotl = dot(normal, lightDir);
	float diff = max(ndotl, 0.0);
	float diffuse  = diff * (1.0 - material.specular) * attenuation;
	// specular
	vec3 viewDir = normalize(cameraPos - worldPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.smoothness);
	float specular = spec * material.specular * attenuation;
	// shadow
	float shadow = ShadowCalculation(ndotl, lightSpaceFragPos) * 0.6;

	return light.color * (ambient * albedo + (1.0 - shadow) * (diffuse * albedo + specular));
}

vec3 GammaCorrection(vec3 color)
{
	return pow(color, vec3(1.0/2.2));
}

void main()
{
	// Note that branching in this shader is OK, because all threads will take one branch or the other together.
	vec3 norm = gl_FrontFacing ? normalize(vs.normal) : -normalize(vs.normal);
	vec3 diffuseColor = material.useTexture ? vec3(texture(material.diffuse, vs.uv)) : material.tint;
	vec3 lighting = CalcSpotLight(spotLight, _CameraPos, norm, vs.worldPos, vs.lightSpaceFragPos, material, diffuseColor);
	FragColor = vec4(GammaCorrection(lighting), 1.0);
}