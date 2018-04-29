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
	vec3 v1 = teposition[1] - teposition[0];
	vec3 v2 = teposition[2] - teposition[0];
	vec3 normal = normalize(cross(v1, v2));
	for(int i = 0; i < 3; i++)
	{
		getex = tetex[i];	
		geposition = teposition[i];
		genormal = tenormal[i];
		//genormal = normal;
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
