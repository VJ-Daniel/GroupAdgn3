#version 330 core

layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 model;

uniform vec2 uvOffset;
uniform vec2 uvScale;

void main()
{
    TexCoords = vertex.zw * uvScale + uvOffset;

    gl_Position = projection * model *
                  vec4(vertex.xy, 0.0, 1.0);
}