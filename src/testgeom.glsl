#version 440 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec2 tetex[];
in vec3 teposition[];
in vec3 tenormal[];


out vec2 getex;
out vec3 geposition;
out vec3 genormal;


void main() 
{	
	for(int i = 0; i < 3; i++)
	{
		getex = tetex[i];	
		geposition = teposition[i];
		genormal = tenormal[i];
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	getex = tetex[0];	
	geposition = teposition[0];
	genormal = tenormal[0];
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	EndPrimitive();
}
