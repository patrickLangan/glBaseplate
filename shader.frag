#version 150 core

out vec4 outColor;
in vec3 pos;

void main ()
{
	//outColor = vec4 (1.0, 1.0, 1.0, 1.0);
	outColor = vec4 ((pos + 1) / 2, 1.0);
}

