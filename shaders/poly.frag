#version 330 core

uniform bool is_mesh;

uniform vec3 eye_pos;
uniform vec3 view_dir;

in vec3 vpos;
in vec3 vnorm;

out vec4 color;

void main() {
    if (is_mesh) {
        color = vec4(0.03, 0.1, 0.93, 1);
    }
    else {
        vec3 light_dir = vec3(0, -1, 0);
        float l1 = max(0, dot(-light_dir, vnorm)) / 2.0f;

        vec3 lDir = normalize(vpos - eye_pos);
        float spotAngle = dot(view_dir, lDir);
        float lit = float(acos(spotAngle) <= radians(75.0f));
        float l2 = lit * max(0, pow(spotAngle, 5)) / 5.0f;

        color = (l1 + l2) * vec4(0.03, 0.1, 0.93, 1);
    }
}
