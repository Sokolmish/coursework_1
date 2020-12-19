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
uniform vec3 globalAmb;
uniform vec3 skyColor;

uniform vec3 baseDim;
uniform vec3 baseBright;

uniform Material mat;

uniform sampler2D normalMap;
uniform sampler2D perlinNoise;

in vec3 vpos;
in vec2 texc;

out vec4 color;

void main() {
    if (is_mesh) {
        color = vec4(mesh_color, 1.0);
    }
    else {
        const float brightTreshold = 0.99;
        vec3 normal = normalize(texture(normalMap, texc).xyz);
        vec3 viewDir = normalize(eye_pos - vpos);
        vec3 halfway = normalize(sunDir + viewDir);

        float cost1 = dot(viewDir, normal);
        float sint2 = 0.75 * sqrt(1.0 - cost1 * cost1);
        float cost2 = 1.5 * sqrt(1 - sint2 * sint2);
        float fresnel = (cost1 - 1.35 * cost2) / (cost1 + 1.35 * cost2);
        fresnel = fresnel * fresnel;

        float ambient = max(0.0, dot(reflect(viewDir, normal), viewDir));
        float diffuse = -min(0.0, dot(normal, sunDir));
        float specular = pow(max(0.0, dot(normal, halfway)), mat.exponent);

        vec3 res;
        if (specular >= brightTreshold)
            res = baseBright;
        else
            res = baseDim +
                ambient * mat.ambient +
                diffuse * fresnel * mat.diffuse +
                specular * fresnel * mat.specular;

        res += skyColor;
        if(res.b < 0.9)
            res *= fresnel;

        color = vec4(res, 1.0);
    }
}
