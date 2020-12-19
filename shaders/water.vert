#version 430 core

uniform mat4 m_proj_view;

uniform float gNodes;

layout (location = 0) in vec3 pos;

out vec3 vpos;
out vec2 texc;

void main() {
    gl_Position = m_proj_view * vec4(pos, 1.0);
    vpos = pos;
    texc = vec2(pos.x / gNodes, pos.z / gNodes);
}
