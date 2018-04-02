#version 440 core

layout(quads, equal_spacing, cw) in;

in vec2 tctex[];

out vec2 tetex;
out vec3 teposition;

uniform sampler2D heightmap;
uniform vec3 scale;
uniform mat4 proj_view;

uniform vec2 patchPos;
uniform float patchSize;


void main() {
	vec2 pos = patchPos + patchSize * gl_TessCoord.xy;
	
	vec2 uv = pos / scale.xz;
	tetex = uv;

	teposition = pos.xyy;
	teposition.y = scale.y * texture(heightmap, uv).r;


	gl_Position = proj_view * vec4(teposition, 1.0);
}