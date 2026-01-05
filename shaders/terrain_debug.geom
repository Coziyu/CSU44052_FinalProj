#version 330 core

layout(points) in;  
layout(line_strip, max_vertices = 2) out; 

uniform mat4 MVP; 

in vec3 fragNorm[];  // normal passed from vertex shader
in vec3 fragPos[]; // position passed from vertex shader

// [ACKN] ChatGPT assisted in creating this geometry shader to visualize terrain normals when debugging terrain generation.
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
