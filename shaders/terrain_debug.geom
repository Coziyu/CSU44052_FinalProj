#version 330 core

layout(points) in;          // Input: points
layout(line_strip, max_vertices = 2) out; // Output: lines

uniform mat4 MVP; // Model-View-Projection matrix

in vec3 fragNorm[];  // normal passed from vertex shader
in vec3 fragPos[]; // position passed from vertex shader

void main() {
    vec3 pos = fragPos[0];
    vec3 norm = fragNorm[0];
    float normalLength = 50.0; // length of normal line

    // First vertex: start of the line (at position)
    gl_Position = MVP * vec4(pos, 1.0);
    EmitVertex();

    // Second vertex: end of the line (position + normal * length)
    gl_Position = MVP * vec4(pos + norm * normalLength, 1.0);
    EmitVertex();

    EndPrimitive();
}
