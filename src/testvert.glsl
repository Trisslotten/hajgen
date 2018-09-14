#version 440 core
layout(location = 0) in vec2 pos;

out vec2 vPatchPos;

uniform float numPatches;
uniform vec3 size;

void main() {
	vPatchPos = size.xz * pos / numPatches;
}