#version 460 core

void main()
{
	// Since we have no color buffer and disabled the draw and read buffers,
	// the resulting fragments do not require any processing so we can simply use an empty fragment shader:
	// This commented line is just what's going on behind the scenes.

	// gl_FragDepth = gl_FragCoord.z;
}