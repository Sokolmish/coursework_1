#version 430 core

#define M_PI 3.14159265358979323846
#define M_1_PI 0.318309886183790671538
#define M_SQRT1_2 0.707106781186547524401

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float exponent;
};

uniform bool is_mesh;
uniform vec3 mesh_color;

uniform vec3 eye_pos;
uniform vec3 sunDir;

uniform Material mat;
uniform vec3 globalAmb;

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

        // vec3 upwelling = vec3(0, 0.2, 0.3); // vec3(0.03f, 0.391f, 0.9993f) * 0.35f
        // vec3 sky       = vec3(0.1f, 0.55f, 0.75f);
        // // vec3 air       = vec3(0.69f, 0.84f, 1.00f);
        // float nSnell    = 1.34;
        // float Kdiffuse  = 0.91;

        // float reflectivity;
        // vec3 nI = normalize(-sunDir);
        // vec3 nN = normalize(normal);
        // float costhetai = abs(dot(nI, nN));
        // float thetai = acos(costhetai);
        // float sinthetat = sin(thetai) / nSnell;
        // float thetat = asin(sinthetat);
        // if (thetai == 0.0) {
        //     reflectivity = (nSnell - 1) / (nSnell + 1);
        //     reflectivity = reflectivity * reflectivity;
        // }
        // else {
        //     float fs = sin(thetat - thetai) / sin(thetat + thetai);
        //     float ts = tan(thetat - thetai) / tan(thetat + thetai);
        //     reflectivity = 0.5 * (fs * fs + ts * ts);
        // }
        // // vec3 dPE = (vpos - eye_pos) / 2000.0;
        // // float dist = length(dPE) * Kdiffuse;
        // // dist = exp(-dist);
        // vec3 Ci = 1.0 * (reflectivity * sky + (1 - reflectivity) * upwelling) ;//+  (1 - dist) * air;

        // color = vec4(Ci, 1.0);
    }
}
