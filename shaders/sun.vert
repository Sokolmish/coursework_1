#version 430 core

uniform mat4 projection;
layout (location = 0) in vec3 vertex;

void main() {
    gl_Position = projection * vec4(vertex, 1.0);
}
