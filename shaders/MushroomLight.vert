#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec4 jointIndices;
layout(location = 4) in vec4 jointWeights; // If !skinning, this is used as local node transform
layout(location = 5) in vec4 tangent;
layout(location = 6) in vec2 vertexUV1;
layout(location = 7) in vec2 vertexUV2;

// Output data, to be interpolated for each fragment
out vec3 worldPosition;
out vec3 worldNormal;
out vec2 fragUV;
out vec4 fragTangent;
out vec2 fragUV1;
out vec2 fragUV2;

uniform mat4 MVP;
uniform mat4 jointMatrices[100];
uniform mat4 nodeMatrix;
uniform bool isSkinned;

void main() {
    
    mat4 skinMat = 
        jointWeights.x * jointMatrices[int (jointIndices.x)] + 
        jointWeights.y * jointMatrices[int (jointIndices.y)] + 
        jointWeights.z * jointMatrices[int (jointIndices.z)] + 
        jointWeights.w * jointMatrices[int (jointIndices.w)];

    
    vec4 worldPosition4 = vec4(vertexPosition, 1.0);
    if (isSkinned) {
        worldPosition4 = skinMat * worldPosition4;
        // Apply node transform after skinning
        worldPosition4 = nodeMatrix * worldPosition4;
        gl_Position = MVP * worldPosition4;
        mat3 normalMat = mat3(nodeMatrix * skinMat);
        worldPosition = worldPosition4.xyz;
        worldNormal = normalize(normalMat * vertexNormal);
        // transform tangent
        vec3 t = normalize(mat3(nodeMatrix * skinMat) * tangent.xyz);
        fragTangent = vec4(t, tangent.w);
    } else {
        worldPosition4 = nodeMatrix * worldPosition4;
        gl_Position = MVP * worldPosition4;
        mat3 normalMat = mat3(nodeMatrix);
        worldPosition = worldPosition4.xyz;
        worldNormal = normalize(normalMat * vertexNormal);
        vec3 t = normalize(mat3(nodeMatrix) * tangent.xyz);
        fragTangent = vec4(t, tangent.w);
    }

    fragUV = vertexUV;
    fragUV1 = vertexUV1;
    fragUV2 = vertexUV2;
}