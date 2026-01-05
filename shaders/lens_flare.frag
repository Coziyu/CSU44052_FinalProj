#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform vec2 lightScreenPos;  // light pos in screen space (0 to 1)
uniform vec2 screenSize;
uniform float intensity;
uniform float scale;
uniform float time;
uniform float aspectRatio;
uniform float streakLength;
uniform float haloRadius;
uniform int numGhosts;

// lens flare ghost
float ghost(vec2 uv, vec2 ghostPos, float size, float falloff) {
    vec2 diff = uv - ghostPos;
    diff.x *= aspectRatio;  // aspect ratio skews the circle into an ellipse
    float dist = length(diff);
    return pow(max(0.0, 1.0 - dist / size), falloff);
}

// halo around light
float halo(vec2 uv, vec2 lightPos, float radius, float thickness) {
    vec2 diff = uv - lightPos;
    diff.x *= aspectRatio;
    float dist = length(diff);
    return smoothstep(radius - thickness, radius, dist) * 
           smoothstep(radius + thickness, radius, dist);
}

// short star streak effect around light
float streak(vec2 uv, vec2 lightPos, float length, float width) {
    vec2 diff = uv - lightPos;
    diff.x *= aspectRatio;
    
    // horizontal streak - limited length
    float hStreak = exp(-abs(diff.y) / width) * exp(-abs(diff.x * aspectRatio) / length);
    
    // vertical streak - limited length  
    float vStreak = exp(-abs(diff.x * aspectRatio) / width) * exp(-abs(diff.y) / length);
    
    return max(hStreak, vStreak);
}

vec3 lensFlare(vec2 uv) {
    vec3 flareColor = vec3(0.0);
    
    vec2 screenCenter = vec2(0.5);
    vec2 dir = lightScreenPos - screenCenter;
    
    // sun glow at light pos
    vec2 diff = uv - lightScreenPos;
    diff.x *= aspectRatio;
    float dist = length(diff);
    float sunGlow = exp(-dist * 8.0) * 1.5;
    flareColor += vec3(1.0, 0.9, 0.6) * sunGlow;
    
    // halo around the sun
    float haloEffect = halo(uv, lightScreenPos, haloRadius * scale, 0.02 * scale);
    flareColor += vec3(0.8, 0.6, 0.3) * haloEffect * 0.5;
    
    float halo2 = halo(uv, lightScreenPos, haloRadius * 1.67 * scale, 0.015 * scale);
    flareColor += vec3(0.5, 0.4, 0.7) * halo2 * 0.3;
    
    // ghosts by reflection across centre
    for (int i = 1; i <= numGhosts; i++) {
        float fi = float(i);
        float ghostScale = (0.6 - fi * 0.1) * scale;
        float ghostPos = -fi * 0.4;  // position along flare line
        vec2 gPos = screenCenter + dir * ghostPos;
        
        float g = ghost(uv, gPos, 0.05 + fi * 0.02, 2.0 + fi * 0.5); // TODO: refactor these magic numbers if have time
        
        vec3 ghostColor;
        if (i == 1) ghostColor = vec3(0.6, 0.3, 0.9);
        else if (i == 2) ghostColor = vec3(0.3, 0.9, 0.6);
        else if (i == 3) ghostColor = vec3(0.9, 0.6, 0.3);
        else if (i == 4) ghostColor = vec3(0.3, 0.6, 0.9);
        else ghostColor = vec3(0.9, 0.3, 0.6);
        
        flareColor += ghostColor * g * (0.3 - fi * 0.04);
    }
    
    // opposite side
    for (int i = 1; i <= 3; i++) {
        float fi = float(i);
        float ghostPos = fi * 0.3;
        vec2 gPos = screenCenter + dir * ghostPos;
        
        float g = ghost(uv, gPos, 0.03 + fi * 0.015, 3.0);
        vec3 ghostColor = vec3(0.5, 0.8, 1.0);
        flareColor += ghostColor * g * 0.2;
    }
    
    // short cross star streak
    float streakEffect = streak(uv, lightScreenPos, streakLength * scale, 0.003 * scale);
    flareColor += vec3(1.0, 0.95, 0.85) * streakEffect * 0.8;
    
    // chrom aberration on edges
    float edgeDist = length(uv - screenCenter) * 2.0;
    vec3 chromatic = vec3(
        ghost(uv, lightScreenPos + dir * 0.02, 0.1, 2.0),
        ghost(uv, lightScreenPos, 0.1, 2.0),
        ghost(uv, lightScreenPos - dir * 0.02, 0.1, 2.0)
    ) * edgeDist * 5.3;
    flareColor += chromatic;
    
    // starburst effect
    float angle = atan(diff.y, diff.x);
    float rays = pow(abs(sin(angle * 8.0 + time * 0.5)), 16.0) * exp(-dist * 5.0);
    flareColor += vec3(1.0, 0.95, 0.8) * rays * 0.4;
    
    return flareColor;
}

void main()
{
    vec3 flare = lensFlare(TexCoords) * intensity;
    
    // distance fade since flare gets weaker when light is near screen edge
    vec2 screenCenter = vec2(0.5);
    float distFromCenter = length(lightScreenPos - screenCenter);
    float edgeFade = 1.0 - smoothstep(0.3, 0.7, distFromCenter);
    
    flare *= edgeFade;
    
    FragColor = vec4(flare, 1.0);
}
