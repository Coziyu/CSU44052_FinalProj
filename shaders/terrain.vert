#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aNorm;

uniform mat4 MVP;
uniform mat4 Model;

out vec4 color;
out vec3 fragPos;
out vec3 fragNorm;
out vec3 worldPosition;

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0);

    color = vec4(1.0, 0.3, 1.0, 1.0);
    fragPos = aPos;
    worldPosition = vec3(Model * vec4(aPos, 1.0));
    fragNorm = mat3(transpose(inverse(Model))) * aNorm; // [ACKN] ChatGPT suggested fix to this line for terrain lighting issues
}