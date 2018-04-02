#version 440 core

out vec4 out_color;

in vec2 tetex;

in vec3 teposition;

uniform sampler2D heightmap;

uniform vec3 scale;


vec3 sampleNormal(vec2 uv) {
	vec2 heightmapSize = vec2(textureSize(heightmap, 0));
	float t = 1/heightmapSize.x;

	vec4 h;
	h[0] = texture(heightmap, uv + t * vec2(0,-1)).r;
	h[1] = texture(heightmap, uv + t * vec2(-1,0)).r;
	h[2] = texture(heightmap, uv + t * vec2(1, 0)).r;
	h[3] = texture(heightmap, uv + t * vec2(0, 1)).r;

	float ratioX = t*scale.x/(scale.y);
	float ratioZ = t*scale.z/(scale.y);
	vec3 n;
	n.x = ratioX * (h[1] - h[2]);
	n.z = ratioZ * (h[0] - h[3]);
	n.y = 2 * ratioX*ratioZ;
	return normalize(n);
}

void main() {
	vec3 normal = sampleNormal(tetex);

	float diffuse = 0.9*max(dot(normal, normalize(vec3(-1,1,0))), 0);
	float ambient = 0.1;
	vec3 color = vec3(1) * (diffuse + ambient);


	out_color = vec4(color, 1.0);
}