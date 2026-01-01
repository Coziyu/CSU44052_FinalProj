#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aNorm;

uniform mat4 MVP;

out vec4 color;
out vec3 fragPos;
out vec3 fragNorm;

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0);

    color = vec4(1.0, 0.3, 1.0, 1.0);
    fragPos = aPos;
    fragNorm = aNorm;
}