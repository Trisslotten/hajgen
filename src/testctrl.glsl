#version 440 core

layout(vertices = 4) out;
in vec2 vtex[];
out vec2 tctex[];

void main() 
{
	tctex[gl_InvocationID] = vtex[gl_InvocationID];

	float divisions = 64.0;
	gl_TessLevelInner[0] = divisions;
	gl_TessLevelInner[1] = divisions;

	gl_TessLevelOuter[0] = divisions;
	gl_TessLevelOuter[1] = divisions;
	gl_TessLevelOuter[2] = divisions;
	gl_TessLevelOuter[3] = divisions;
}