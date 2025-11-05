#version 330 core

in vec4 fragColor;       // vine din vertex shader
out vec4 outColor;

uniform float colorFactor; // pentru intunecare

void main(void)
{
    outColor = vec4(fragColor.rgb * colorFactor, fragColor.a);
}
