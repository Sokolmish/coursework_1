#version 430 core

uniform mat4 m_proj_view;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

out vec3 vpos;
out vec3 vnorm;

void main() {
    gl_Position = m_proj_view * vec4(pos, 1.0);
    vpos = pos;
    vnorm = norm;
}
