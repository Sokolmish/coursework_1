#version 430 core

uniform vec3 sunColor;
out vec4 color;

void main() {
    color = vec4(sunColor, 1.0);
}