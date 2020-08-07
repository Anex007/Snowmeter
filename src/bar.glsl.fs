#version 400 core

// #define NUM_BARS 4
#define MAX_BARS 64

in vec4 gl_FragCoord;

out vec4 FragColor;

// you can use this information to color grade based on the height.
uniform float u_Heights[MAX_BARS];
uniform uint  u_NumBars;
uniform vec3  u_BaseGradient;
uniform vec3  u_MaxGradient;
uniform ivec2 u_Resolution;

void main()
{
    float height = gl_FragCoord.y / u_Resolution.y;
    FragColor = vec4(u_BaseGradient + u_MaxGradient * height, 1.0);
}
