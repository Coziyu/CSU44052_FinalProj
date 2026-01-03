#version 330 core

in vec3 worldPosition;
in vec3 worldNormal; 
in vec2 fragUV;
in vec4 fragTangent;
in vec2 fragUV1;
in vec2 fragUV2;

out vec4 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 cameraPos;
uniform samplerCube shadowCubemap;
uniform float farPlane;

// Material textures (samplers)
uniform sampler2D baseColorTex;
uniform sampler2D metallicRoughnessTex;
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

// UV set selection (0 = TEXCOORD_0, 1 = TEXCOORD_1, 2 = TEXCOORD_2)
uniform int baseColorUV;
uniform int mrUV;
uniform int normalUV;
uniform int occlusionUV;
uniform int emissiveUV;

// Convert sRGB to linear space
vec3 sRGBToLinear(vec3 srgb)
{
    return pow(srgb, vec3(2.2));
}

// Helper to select UV set
vec2 getUV(int set) {
    return fragUV; // Hack fix???
    if (set == 0) return fragUV;
    if (set == 1) return fragUV1;
    if (set == 2) return fragUV2;
}

float calculateShadow(vec3 fragPos)
{
    vec3 lightToFrag = fragPos - lightPosition;
    float currentDepth = length(lightToFrag);
    
    float closestDepth = texture(shadowCubemap, lightToFrag).r;
    closestDepth *= farPlane;
    
    float bias = max(0.05, 0.005 * currentDepth);
    float shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
    
    return 1.0; // DOESN'T WORK??????
}

void main()
{
    // Fetch material properties
    vec3 albedo = u_BaseColorFactor.rgb;
    float alpha = u_BaseColorFactor.a;
    float metallic = u_MetallicFactor;
    float roughness = clamp(u_RoughnessFactor, 0.05, 1.0);
    vec3 emissive = u_EmissiveFactor;
    float ao = 1.0;

    if (hasBaseColorTex) {
        vec4 c = texture(baseColorTex, getUV(baseColorUV));
        albedo *= sRGBToLinear(c.rgb);
        alpha *= c.a;
    }
    
    if (hasMetallicRoughnessTex) {
        vec4 mr = texture(metallicRoughnessTex, getUV(mrUV));
        metallic *= mr.b;
        roughness *= mr.g;
    }
    
    if (hasEmissiveTex) {
        vec3 em = texture(emissiveTex, getUV(emissiveUV)).rgb;
        emissive *= sRGBToLinear(em);
    }
    
    if (hasOcclusionTex) {
        ao = mix(1.0, texture(occlusionTex, getUV(occlusionUV)).r, u_OcclusionStrength);
    }

    // Discard fragments with low alpha
    if (alpha < 0.1) {
        discard;
    }

    // Normal mapping
    vec3 N = normalize(worldNormal);
    if (hasNormalTex) {
        vec3 T = normalize(fragTangent.xyz);
        vec3 B = cross(worldNormal, T) * fragTangent.w;
        mat3 TBN = mat3(T, B, normalize(worldNormal));
        vec3 nmap = texture(normalTex, getUV(normalUV)).rgb;
        nmap = nmap * 2.0 - 1.0;
        N = normalize(TBN * nmap);
    }

    // View and light vectors
    vec3 V = normalize(cameraPos - worldPosition);
    vec3 L = normalize(lightPosition - worldPosition);
    vec3 H = normalize(V + L);

    // Fresnel (Schlick)
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float NdotV = max(dot(N, V), 0.001);
    float NdotL = max(dot(N, L), 0.0);

    // Distribution (GGX)
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    float D = a2 / (3.14159265 * denom * denom + 1e-6);

    // Geometry (Schlick-GGX)
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float Gv = NdotV / (NdotV * (1.0 - k) + k);
    float G_l = NdotL / (NdotL * (1.0 - k) + k);
    float G = Gv * G_l;

    vec3 F = F0 + (1.0 - F0) * pow(1.0 - max(dot(H, V), 0.0), 5.0);

    vec3 numerator = D * G * F;
    float denomBRDF = 4.0 * NdotV * NdotL + 1e-6;
    vec3 specular = numerator / denomBRDF;

    vec3 kD = (1.0 - F) * (1.0 - metallic);

    float distToLight = length(lightPosition - worldPosition);
    vec3 radiance = lightIntensity / (distToLight * distToLight + 0.1);

    // Calculate shadow
    float shadow = calculateShadow(worldPosition);
    
    vec3 Lo = (kD * albedo / 3.14159265 + specular) * radiance * NdotL * shadow;

    // Ambient
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo + emissive;

    // Tone mapping + gamma
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    finalColor = vec4(color, alpha);
}
