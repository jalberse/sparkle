#version 460 core

layout (location = 0) in vec3 aPos;
uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;

void main()
{
	gl_Position = _Projection * _View * _Model * vec4(aPos, 1.0);
}