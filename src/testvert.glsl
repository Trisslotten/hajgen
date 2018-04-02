#version 440 core
layout(location = 0) in vec2 tex;

out vec2 vtex;

void main() {
	vtex = tex;
}