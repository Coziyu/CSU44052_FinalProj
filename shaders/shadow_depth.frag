#version 330 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;
uniform vec3 cameraPos;
uniform float viewDistance;
uniform float fadeDistance;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    lightDistance = lightDistance / farPlane;
    
    // only cast shadows when object is at least halfway faded in
    float distToCamera = length(FragPos.xyz - cameraPos);
    float fadeStart = viewDistance - fadeDistance;
    float fadeMidpoint = fadeStart + fadeDistance * 0.5;  // halfway point
    
    if (distToCamera > fadeMidpoint) {
        // dont cast shadow yet
        gl_FragDepth = 1.0;
        return;
    }
    
    gl_FragDepth = lightDistance;
}
