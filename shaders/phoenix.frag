#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 fragUV;
in vec4 fragTangent;
in vec2 fragUV1;
in vec2 fragUV2;

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform samplerCube shadowCubemap;
uniform float farPlane;

// Material textures (samplers)
uniform sampler2D baseColorTex;
uniform sampler2D metallicRoughnessTex; // R=metallic, G=roughness (glTF packing varies)
uniform sampler2D normalTex;
uniform sampler2D occlusionTex;
uniform sampler2D emissiveTex;

// Flags
uniform bool hasBaseColorTex;
uniform bool hasMetallicRoughnessTex;
uniform bool hasNormalTex;
uniform bool hasOcclusionTex;
uniform bool hasEmissiveTex;

// Material factors
uniform vec4 u_BaseColorFactor;
uniform float u_MetallicFactor;
uniform float u_RoughnessFactor;
uniform vec3 u_EmissiveFactor;
uniform float u_OcclusionStrength;
// Which UV set to use for each texture (0 = TEXCOORD_0, 1 = TEXCOORD_1, 2 = TEXCOORD_2)
uniform int baseColorUV;
uniform int mrUV;
uniform int normalUV;
uniform int occlusionUV;
uniform int emissiveUV;

// Helper to select UV set
vec2 getUV(int set) {
	if (set == 0) return fragUV;
	if (set == 1) return fragUV1;
	if (set == 2) return fragUV2;
	return fragUV;
}

float calculateShadow(vec3 fragPos)
{
    vec3 lightToFrag = fragPos - lightPosition;
    float currentDepth = length(lightToFrag);
    
    float closestDepth = texture(shadowCubemap, lightToFrag).r;
    closestDepth *= farPlane;  // Back to real distance
    
    // Adaptive bias: scales with distance to reduce artifacts at far distances
    float bias = max(0.005 * currentDepth, 0.1);
    
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    
    // return 1.0 - shadow;
	return 1.0; // DOESN'T WORK??????
}

void main()
{

	// Base color / albedo
	vec3 albedo = u_BaseColorFactor.rgb;
	float alpha = u_BaseColorFactor.a;
	if (hasBaseColorTex) {
		vec3 t = texture(baseColorTex, getUV(baseColorUV)).rgb;
		// textures are typically stored in sRGB; convert to linear for lighting
		t = pow(t, vec3(2.2));
		albedo *= t;
	}

	// Occlusion
	float occlusion = 1.0;
	if (hasOcclusionTex) {
		float occ = texture(occlusionTex, getUV(occlusionUV)).r;
		occlusion = mix(1.0, occ, u_OcclusionStrength);
	}

	// Emissive
	vec3 emissive = u_EmissiveFactor;
	if (hasEmissiveTex) {
		vec3 te = texture(emissiveTex, getUV(emissiveUV)).rgb;
		te = pow(te, vec3(2.2));
		emissive *= te;
	}

	// Normal mapping (TBN)
	vec3 N = normalize(worldNormal);
	if (hasNormalTex) {
		vec3 nmap = texture(normalTex, getUV(normalUV)).xyz;
		nmap = nmap * 2.0 - 1.0;
		vec3 T = normalize(fragTangent.xyz);
		vec3 B = normalize(cross(N, T) * fragTangent.w);
		mat3 TBN = mat3(T, B, N);
		N = normalize(TBN * nmap);
	}

	// Lighting (simple Lambertian diffuse with distance falloff)
	vec3 lightDir = lightPosition - worldPosition;
	float lightDist = dot(lightDir, lightDir);
	lightDir = normalize(lightDir);
	float lambert = clamp(dot(lightDir, N), 0.0, 1.0);
	
	// Calculate shadow
	float shadow = calculateShadow(worldPosition);
	
	vec3 v = lightIntensity * lambert * shadow / lightDist;
	v = v / (1.0 + v); // tone mapping

	vec3 color = albedo * v * occlusion + emissive;
	finalColor = pow(color, vec3(1.0 / 2.2));
}
