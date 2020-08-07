#version 400 core

/*
uniform vec2 resolution;
uniform vec2 center;
uniform float range;
*/

//uniform dvec2 resolution;

//in vec4 gl_FragCoord;

out vec4 FragColor;

uniform vec4 u_Color;

void main()
{
	//FragColor = vec4(0.5, 0.2, 0.5, 0.0);
	FragColor = u_Color;
}
