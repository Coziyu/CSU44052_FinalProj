#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec3 aNorm;

uniform mat4 Model;

void main()
{
    gl_Position = Model * vec4(aPos, 1.0);
}
