#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec3 aNorm;
layout(location = 3) in vec4 jointIndices;
layout(location = 4) in vec4 jointWeights;

uniform mat4 Model;  // position/scale/rotation
uniform mat4 nodeMatrix;  // per-node transform for mesh hierarchy
uniform mat4 jointMatrices[100];  // bone transforms for animation
uniform bool isSkinned;  // is this a skeletal model?

void main()
{
    vec4 worldPos = vec4(aPos, 1.0);
    
    if (isSkinned) {
        // Skinned model: apply bone transforms then node matrix
        mat4 skinMat = 
            jointWeights.x * jointMatrices[int(jointIndices.x)] + 
            jointWeights.y * jointMatrices[int(jointIndices.y)] + 
            jointWeights.z * jointMatrices[int(jointIndices.z)] + 
            jointWeights.w * jointMatrices[int(jointIndices.w)];
        worldPos = skinMat * worldPos;
    }
    
    // Apply node matrix for mesh hierarchy (used by both skinned and non-skinned)
    worldPos = nodeMatrix * worldPos;
    
    // Apply the model transform (position/scale/rotation of the entity)
    gl_Position = Model * worldPos;
}
