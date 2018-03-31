#version 440 core

uniform sampler2D texSampler;

out vec4 out_color;

in vec2 Tex;

void main() {
	vec4 color = texture(texSampler, vec2(Tex.x, 1 - Tex.y));
	
	out_color = vec4(color.rrr, 1.0);
}