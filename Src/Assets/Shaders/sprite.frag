#version 330 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D spriteTexture;
uniform vec4 spriteColor;

void main()
{
    FragColor =
        texture(spriteTexture, TexCoords) *
        spriteColor;
}