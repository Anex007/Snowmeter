#version 400 core

in vec4 gl_FragCoord;

out vec4 FragColor;

uniform float u_Offset;
uniform ivec2 u_Resolution;

void main()
{
	float x = gl_FragCoord.x / u_Resolution.x;
	vec3 amp = vec3(0.7, x, 0.9);
	float f;
	if (u_Offset > 0.0)
		f = 0.5;
	else
		f = 0.0;
	amp *= ((gl_FragCoord.y / u_Resolution.y) - f) * 3.0;
	FragColor = vec4(amp, 1.0);
}
