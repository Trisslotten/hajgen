#version 440 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 proj_view;
uniform vec3 cameraPos;
uniform sampler2D heightmap;
uniform vec3 size;
uniform vec2 hmPos;

uniform sampler2D texTree;

in vec2 vPos[];

out vec2 uv;

#define HASHSCALE1 .1031
float hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

vec3 sampleNormal(vec2 uv) {
	vec2 heightmapSize = vec2(textureSize(heightmap, 0));
	float t = 2.0/heightmapSize.x;

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

void main() 
{
	vec3 pos;
	pos.xz = vPos[0] + size.xz * hmPos;
	vec2 hmUv = pos.xz / size.xz - hmPos;
	float t = 1.5/textureSize(heightmap, 0).x;
	hmUv = (1.0 - 2.0*t)*hmUv + t;
	pos.y = size.y * texture(heightmap, hmUv).r - 0.5;


	vec3 normal = sampleNormal(hmUv);
	if(normal.y > 0.9) 
	{
		vec3 lUp = vec3(0,1,0);
		vec3 look;
		look.y = 0;
		look.xz = normalize(cameraPos.xz - pos.xz);

		vec3 right = -cross(lUp, look);
		vec3 up = -cross(look, right);

		vec2 texSize = textureSize(texTree, 0);
		float aspect = texSize.x / texSize.y;
		float size = 16.0 + hash12(pos.xz) * 8;
		pos.y += size;
		right *= size * aspect;
		up *= size;

		vec3 vert1 = pos - (right + up);
		gl_Position = proj_view * vec4(vert1, 1.0);
		uv  = vec2(1,1);
		EmitVertex();


		vec3 vert2 = pos - (right - up);
		gl_Position = proj_view * vec4(vert2, 1.0);
		uv  = vec2(1,0);
		EmitVertex();


		vec3 vert3 = pos + (right - up);
		gl_Position = proj_view * vec4(vert3, 1.0);
		uv  = vec2(0,1);
		EmitVertex();

	
		vec3 vert4 = pos + (right + up);
		gl_Position = proj_view * vec4(vert4, 1.0);
		uv = vec2(0,0);
		EmitVertex();


		EndPrimitive();  
	}
}