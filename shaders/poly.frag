#version 330 core

uniform bool is_mesh;

in vec3 vpos;
in vec3 vnorm;

out vec4 color;

void main() {
    if (is_mesh)
        color = vec4(0.03, 0.1, 0.93, 1);
    else
        color = vec4(abs(vnorm.rbg), 1);
}
