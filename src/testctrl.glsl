#version 440 core

layout(vertices = 1) out;
in vec2 vPatchPos[];
out vec2 tcPatchPos[];

uniform sampler2D heightmap;
uniform float numPatches;
uniform vec3 size;
uniform vec2 hmPos;
uniform float patchSize;

uniform mat4 proj_view;
uniform float fov;
uniform vec3 cameraPos;
uniform vec2 windowSize;

const float pixelsPerQuad = 5.0;


// opengl insights modified
bool vertInFrustum(vec4 p) 
{
	return abs(p.x) < p.w && p.z > 0 && p.z < p.w;
}


float level(vec2 offset) 
{
	vec2 pos = vPatchPos[0] + 0.5*patchSize + patchSize * offset;
	pos += size.xz * hmPos;
	float dist = length(vec3(pos.x, 0, pos.y) - cameraPos);
	float b = dist*tan(fov/2.0);
	float ratio = patchSize/(2.0*b);
	float pixels = windowSize.x * ratio;
	float res = pixels/pixelsPerQuad;
	//res = pow(res, 2.f);
	return max(pow(2.f, ceil(log(res)/log(2))), 1.f);
}

void main() 
{


	gl_TessLevelOuter[0] = 0.0;
	gl_TessLevelOuter[1] = 0.0;
	gl_TessLevelOuter[2] = 0.0;
	gl_TessLevelOuter[3] = 0.0;
	
	gl_TessLevelInner[0] = 0.0;
	gl_TessLevelInner[1] = 0.0;


	vec2 pos = vPatchPos[0] + size.xz * hmPos;
	vec2 corners[4];
	corners[0] = pos;
	corners[1] = pos + patchSize * vec2(0,1);
	corners[2] = pos + patchSize * vec2(1,0);
	corners[3] = pos + patchSize * vec2(1,1);

	vec4 transformed[4];
	for(int i = 0; i < 4; i++) 
	{
		transformed[i] = proj_view * vec4(corners[i].x, 0.0, corners[i].y, 1.0);
	}

	if(vertInFrustum(transformed[0]) || 
	   vertInFrustum(transformed[1]) || 
	   vertInFrustum(transformed[2]) || 
	   vertInFrustum(transformed[3])) 
	{
		
		tcPatchPos[gl_InvocationID] = vPatchPos[gl_InvocationID];

	
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
}