#version 440 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;

out vec2 Tex;

void main() {
	Tex = tex;
	gl_Position = vec4(pos, 1);
}