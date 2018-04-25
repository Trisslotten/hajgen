#version 440 core

layout(vertices = 4) out;
in vec2 vtex[];
out vec2 tctex[];

uniform float numPatches;
uniform vec3 size;
uniform vec2 hmPos;
uniform float patchSize;
uniform vec2 patchPos;

uniform mat4 proj_view;
uniform float fov;
uniform vec3 cameraPos;
uniform vec2 windowSize;

const float pixelsPerQuad = 20.0;

float level(vec2 offset) 
{
	vec2 pos = patchPos + 0.5*patchSize + patchSize * offset;
	pos += size.xz * hmPos;	
	float dist = length(vec3(pos.x, 0, pos.y) - cameraPos);
	float b = dist*tan(fov/2.0);
	float ratio = patchSize/(2.0*b);
	float pixels = windowSize.x * ratio;
	float res = pixels/pixelsPerQuad;
	return pow(2, ceil(log(res)/log(2)));
}

void main() 
{
	tctex[gl_InvocationID] = vtex[gl_InvocationID];

	
	float currLevel = level(vec2(0));


	gl_TessLevelOuter[0] = currLevel;
	gl_TessLevelOuter[1] = currLevel;
	gl_TessLevelOuter[2] = currLevel;
	gl_TessLevelOuter[3] = currLevel;


	float left = level(vec2(-1,0));
	if(left < currLevel) {
		gl_TessLevelOuter[0] = left;
	}

	float bottom = level(vec2(0,-1));
	if(bottom < currLevel) {
		gl_TessLevelOuter[1] = bottom;
	}

	float right = level(vec2(1,0));
	if(right < currLevel) {
		gl_TessLevelOuter[2] = right;
	}

	float top = level(vec2(0,1));
	if(top < currLevel) {
		gl_TessLevelOuter[3] = top;
	}


	
	gl_TessLevelInner[0] = currLevel;
	gl_TessLevelInner[1] = currLevel;
}