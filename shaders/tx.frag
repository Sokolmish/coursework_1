#version 430 core

uniform sampler2D tex;
in vec2 TexCoords;
out vec4 color;

void main() {
    color = texture(tex, TexCoords);
}
