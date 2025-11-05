#version 330 core

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec4 in_Color;

out vec4 fragColor;  // iese din vertex shader spre fragment shader
uniform mat4 myMatrix;

void main(void)
{
    gl_Position = myMatrix * in_Position;
    fragColor = in_Color; // trimite culoarea catre fragment shader
}
