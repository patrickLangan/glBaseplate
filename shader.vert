#version 150 core

in vec3 position;
out vec3 pos;

void main ()
{
	pos = position;
	gl_Position = vec4(position / 2, 1.0);
}

