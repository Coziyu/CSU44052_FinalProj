#version 330 core

in vec4 color;
in vec3 fragPos;
in vec3 fragNorm;
in vec3 worldPosition;

uniform vec3 lightPosition;
uniform samplerCube shadowCubemap;
uniform float farPlane;

out vec4 finalColor;

vec3 reinhard_tone_mapper(vec3 v){
    return v / (1.0 + v);
}

vec3 gamma_correct(vec3 v, float g){
    return pow(v, vec3(1.0 / g));
}

float calculateShadow(vec3 fragPos)
{
    vec3 lightToFrag = fragPos - lightPosition;
    float currentDepth = length(lightToFrag);
    
    // ChatGPT assisted with the bias implementation for shadow ache
    float closestDepth = texture(shadowCubemap, lightToFrag).r;
    closestDepth *= farPlane;  // Back to real distance
    
    // Adaptive bias: scales with distance to reduce artifacts at far distances
    float bias = max(0.005 * currentDepth, 0.1);  // Min 0.1, scales with distance
    
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    
    return 1.0 - shadow;
}

void main()
{
    vec3 ambient = vec3(0.1, 0.1, 0.1);

    vec3 norm = normalize(fragNorm);
    vec3 lightDir = lightPosition - worldPosition;  // direction from fragment to light
    float lightDist = dot(lightDir, lightDir);  // squared distance
    lightDir = normalize(lightDir);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    // Calculate shadow
    float shadow = calculateShadow(worldPosition);
    
    vec3 result = ambient + diffuse * shadow;
    result = result * color.rgb;
    result = reinhard_tone_mapper(result);
    result = gamma_correct(result, 2.2);

    finalColor = vec4(result, 1.0);
}
