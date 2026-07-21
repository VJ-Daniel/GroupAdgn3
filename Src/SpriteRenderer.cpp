#include "Headers/SpriteRenderer.h"

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

SpriteRenderer::SpriteRenderer()
    : shader(nullptr),
    quadVAO(0),
    quadVBO(0)
{}

SpriteRenderer::~SpriteRenderer()
{
    if (quadVBO != 0)
    {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }

    if (quadVAO != 0)
    {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
}

bool SpriteRenderer::Initialize(Shader* shader)
{
    if (shader == nullptr)
    {
        return false;
    }

    this->shader = shader;

    InitializeRenderData();

    this->shader->Use();

    // Tell the sampler uniform to use texture unit 0.
    GLint spriteLocation = glGetUniformLocation(
        this->shader->GetID(),
        "spriteTexture"
    );

    if (spriteLocation != -1)
    {
        glUniform1i(spriteLocation, 0);
    }

    return quadVAO != 0 && quadVBO != 0;
}

void SpriteRenderer::InitializeRenderData()
{
    float vertices[] = {
        // Position       // UV coordinates

        0.0f, 1.0f,      0.0f, 1.0f,
        1.0f, 0.0f,      1.0f, 0.0f,
        0.0f, 0.0f,      0.0f, 0.0f,

        0.0f, 1.0f,      0.0f, 1.0f,
        1.0f, 1.0f,      1.0f, 1.0f,
        1.0f, 0.0f,      1.0f, 0.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        4,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        reinterpret_cast<void*>(0)
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SpriteRenderer::DrawSprite(
    GLuint textureID,
    const glm::vec2& position,
    const glm::vec2& size,
    float rotation,
    const glm::vec4& color,
    const glm::vec2& uvOffset,
    const glm::vec2& uvScale)
{
    if (shader == nullptr || quadVAO == 0 || textureID == 0)
    {
        return;
    }

    shader->Use();

    glm::mat4 model(1.0f);

    // Move the sprite to its screen position.
    model = glm::translate(
        model,
        glm::vec3(position, 0.0f)
    );

    // Move the origin to the sprite's center before rotating.
    model = glm::translate(
        model,
        glm::vec3(
            size.x * 0.5f,
            size.y * 0.5f,
            0.0f
        )
    );

    model = glm::rotate(
        model,
        glm::radians(rotation),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    // Move the origin back after rotation.
    model = glm::translate(
        model,
        glm::vec3(
            -size.x * 0.5f,
            -size.y * 0.5f,
            0.0f
        )
    );

    // Scale the 1-by-1 quad to the requested sprite size.
    model = glm::scale(
        model,
        glm::vec3(size, 1.0f)
    );

    GLint modelLocation = glGetUniformLocation(
        shader->GetID(),
        "model"
    );

    if (modelLocation != -1)
    {
        glUniformMatrix4fv(
            modelLocation,
            1,
            GL_FALSE,
            glm::value_ptr(model)
        );
    }

    GLint colorLocation = glGetUniformLocation(
        shader->GetID(),
        "spriteColor"
    );

    if (colorLocation != -1)
    {
        glUniform4fv(
            colorLocation,
            1,
            glm::value_ptr(color)
        );
    }

    GLint offsetLocation = glGetUniformLocation(
        shader->GetID(),
        "uvOffset"
    );

    if (offsetLocation != -1)
    {
        glUniform2fv(
            offsetLocation,
            1,
            glm::value_ptr(uvOffset)
        );
    }

    GLint scaleLocation = glGetUniformLocation(
        shader->GetID(),
        "uvScale"
    );

    if (scaleLocation != -1)
    {
        glUniform2fv(
            scaleLocation,
            1,
            glm::value_ptr(uvScale)
        );
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}