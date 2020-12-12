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

in vec3 vpos;
in vec2 texc;

out vec4 color;

float rand(vec2 c){
	return fract(sin(dot(c.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(vec2 p, float freq){
	float unit = 1.0 / freq;
	vec2 ij = floor(p / unit);
	vec2 xy = mod(p, unit) / unit;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
	xy = .5 * (1. - cos(3.1415 * xy));
	float a = rand((ij + vec2(0.,0.)));
	float b = rand((ij + vec2(1.,0.)));
	float c = rand((ij + vec2(0.,1.)));
	float d = rand((ij + vec2(1.,1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}

float pNoise(vec2 p, int res){
	float persistance = .56;
	float n = 0.;
	float normK = 0.;
	float f = 39.;
	float amp = 1.;
	int iCount = 0;
	for (int i = 0; i < 50; i++){
		n += amp * noise(p, f);
		f *= 2.;
		normK += amp;
		amp *= persistance;
		if (iCount == res) break;
		iCount++;
	}
	float nf = n/normK;
	return nf*nf*nf*nf;
}

void main() {
    if (is_mesh) {
        color = vec4(mesh_color, 1);
    }
    else {
        vec3 normal = texture(normalMap, texc).xyz;
        vec3 lDir = normalize(sunDir);
        float lIntensivity = max(0, dot(lDir, normal));
         
        vec3 bNoise = vec3(0, 0, pNoise(texc, 50) - 0.3);

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
