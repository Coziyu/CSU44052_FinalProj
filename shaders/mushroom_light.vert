#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec4 jointIndices;
layout(location = 4) in vec4 jointWeights;


// Output data, to be interpolated for each fragment
out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 MVP;
uniform mat4 jointMatrices[100];
uniform bool isSkinned;

void main() {
    mat4 skinMat = 
        jointWeights.x * jointMatrices[int (jointIndices.x)] + 
        jointWeights.y * jointMatrices[int (jointIndices.y)] + 
        jointWeights.z * jointMatrices[int (jointIndices.z)] + 
        jointWeights.w * jointMatrices[int (jointIndices.w)];

    
    vec4 worldPosition4 = isSkinned ? skinMat * vec4(vertexPosition, 1.0) : vec4(vertexPosition, 1.0);

    // Transform vertex
    gl_Position =  MVP * worldPosition4;

    // Get rid of the translation part of this matrix.
    mat3 normalMat = isSkinned ? mat3(skinMat) : mat3(1.0);

    // World-space geometry 
    worldPosition = worldPosition4.xyz;
    worldNormal = normalize(normalMat * vertexNormal);
}
