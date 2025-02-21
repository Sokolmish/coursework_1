#version 430 core

uniform mat4 projection;
layout (location = 0) in vec4 vertex;
out vec2 TexCoords;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
