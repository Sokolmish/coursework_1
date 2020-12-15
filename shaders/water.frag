#version 430 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float exponent;
};

uniform bool is_mesh;
uniform vec3 mesh_color;

uniform vec3 eye_pos;
uniform vec3 view_dir;

uniform Material mat;
uniform vec3 globalAmb;
uniform vec3 sunDir;

uniform sampler2D normalMap;
uniform sampler2D perlinNoise;

in vec3 vpos;
in vec2 texc;

out vec4 color;

void main() {
    if (is_mesh) {
        color = vec4(mesh_color, 1);
    }
    else {
        vec3 normal = texture(normalMap, texc).xyz;
        vec3 lDir = normalize(sunDir);
        float lIntensivity = max(0, dot(lDir, normal));
         
        vec3 bNoise = vec3(0, 0, texture(perlinNoise, texc) - 0.3);

        // Ambient
        vec3 ambient = globalAmb * (mat.ambient);
        // Diffuse
        float diff = max(dot(normal, lDir), 0);
        vec3 diffuse = lIntensivity * (diff * (mat.diffuse + bNoise * 0.3)); // * spotlight.col
        // Specular
        vec3 halfway = normalize(lDir + normalize(eye_pos - vpos));
        float spec = pow(max(dot(normal, halfway), 0), mat.exponent);
        vec3 specular = lIntensivity * (spec * mat.specular); //  * spotlight.col

        color = vec4(ambient + diffuse + specular, 1.0);
    }
}
