#version 440 core
layout(location = 0) in vec2 pos;

out vec2 vPos;

void main() 
{
	vPos = pos;
}