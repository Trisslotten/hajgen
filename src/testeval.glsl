#version 440 core

layout(quads, equal_spacing, cw) in;

in vec2 tctex[];

out vec2 tetex;
out vec3 teposition;

flat out vec3 tenormal;

uniform sampler2D heightmap;
uniform vec3 size;
uniform mat4 proj_view;


uniform vec2 patchPos;
uniform float patchSize;
uniform vec2 hmPos;


vec3 sampleNormal(vec2 uv) {
	vec2 heightmapSize = vec2(textureSize(heightmap, 0));
	float t = 0.5/heightmapSize.x;

	vec4 h;
	h[0] = texture(heightmap, uv + t * vec2(0,-1)).r;
	h[1] = texture(heightmap, uv + t * vec2(-1,0)).r;
	h[2] = texture(heightmap, uv + t * vec2(1, 0)).r;
	h[3] = texture(heightmap, uv + t * vec2(0, 1)).r;

	float ratioX = t*size.x/(size.y);
	float ratioZ = t*size.z/(size.y);
	vec3 n;
	n.x = ratioX * (h[1] - h[2]);
	n.z = ratioZ * (h[0] - h[3]);
	n.y = 2 * ratioX*ratioZ;
	return normalize(n);
}

void main() {
	vec2 pos = patchPos + patchSize * gl_TessCoord.xy;
	
	vec2 uv = pos / size.xz;
	tetex = uv;

	teposition = pos.xyy;
	teposition.y = size.y * texture(heightmap, uv).r;
	teposition.xz += size.xz * hmPos;

	tenormal = sampleNormal(uv);

	gl_Position = proj_view * vec4(teposition, 1.0);
}
