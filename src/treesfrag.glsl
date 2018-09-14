#version 440 core

out vec4 out_color;

uniform sampler2D texTree;

in vec2 uv;

void main() 
{
	vec4 color = texture(texTree, uv);
	if(color.a <= 128.0/255.0) {
		discard;
	}
	//out_color = vec4(color.rgb, 1.0);
	out_color = color;

}