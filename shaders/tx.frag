#version 430 core

uniform sampler2D tex;
in vec2 TexCoords;
out vec4 color;

void main() {
    color = vec4(texture(tex, TexCoords).xyz / 1.0, 1);
}
