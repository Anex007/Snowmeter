#version 400 core

#define MAX_BARS 64

layout(location = 0) in vec2 a_Pos;

uniform float u_Heights[MAX_BARS];
uniform int  u_NumBars;
uniform float u_Offset;

in int gl_DrawID;
in int gl_VertexID;

void main()
{
    // This adds the spacing between bars by checking stuff with drawID and vertexID
    float off = -u_Offset;
    int maxIndex = ((u_NumBars + 1) * 2) - 1;
    if (gl_DrawID == gl_VertexID || gl_DrawID == (maxIndex-gl_VertexID))
        off *= -1.0;

    if (gl_VertexID <= u_NumBars)
        gl_Position = vec4(a_Pos.x + off, -1.0, 0.0, 1.0);
    else
        gl_Position = vec4(a_Pos.x + off, -1.0 + u_Heights[gl_DrawID], 0.0, 1.0);
}