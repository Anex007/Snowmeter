#version 400 core
layout(location = 0) in vec2 a_Pos;

uniform float u_Offset;
uniform float u_MidSample;

// #define MID_SAMPLE 512.0

void main()
{
    float x = -1.0 +  gl_VertexID / u_MidSample;
    if (u_Offset > 0.0)
        gl_Position = vec4(x, u_Offset + (a_Pos.x * 0.5), 0.0, 1.0);
    else
        gl_Position = vec4(x, u_Offset + (a_Pos.y * 0.5), 0.0, 1.0);
}