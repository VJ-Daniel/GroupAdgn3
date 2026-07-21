#pragma once

#include <glew.h>
#include <glm.hpp>

#include "Shader.h"

class SpriteRenderer
{
public:
    SpriteRenderer();
    ~SpriteRenderer();

    bool Initialize(Shader* shader);

    void DrawSprite(
        GLuint textureID,
        const glm::vec2& position,
        const glm::vec2& size,
        float rotation = 0.0f,
        const glm::vec4& color = glm::vec4(1.0f),
        const glm::vec2& uvOffset = glm::vec2(0.0f),
        const glm::vec2& uvScale = glm::vec2(1.0f)
    );

private:
    Shader* shader;

    GLuint quadVAO;
    GLuint quadVBO;

    void InitializeRenderData();
};