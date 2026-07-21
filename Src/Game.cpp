#include <iostream>
#include "Game.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>   

glm::vec4 PixelRegionToUV(
    float x,
    float y,
    float width,
    float height,
    float textureWidth,
    float textureHeight)
{
    float uOffset = x / textureWidth;
    float vOffset = y / textureHeight;

    float uScale = width / textureWidth;
    float vScale = height / textureHeight;

    return glm::vec4(
        uOffset,
        vOffset,
        uScale,
        vScale
    );
}

Game::Game(
    GLFWwindow* gameWindow,
    int width,
    int height)
    : window(gameWindow),
    screenWidth(width),
    screenHeight(height),
    player(
        glm::vec2(100.0f, 500.0f),
        glm::vec2(200.0f, 128.0f),
        250.0f
    )
{}

//--------------------------------------------------
// Initialization
//--------------------------------------------------

bool Game::Initialize(Shader* shader)
{

    if (!spriteRenderer.Initialize(shader))
    {
        return false;
    }

    glm::mat4 projection = glm::ortho(
        0.0f,
        static_cast<float>(screenWidth),
        static_cast<float>(screenHeight),
        0.0f,
        -1.0f,
        1.0f
    );

    shader->Use();

    glUniformMatrix4fv(
        glGetUniformLocation(
            shader->GetID(),
            "projection"
        ),
        1,
        GL_FALSE,
        glm::value_ptr(projection)
    );

    if (!backgroundTexture.LoadFromFile(
        "Src/Assets/Textures/background.png"))
    {
        return false;
    }

    if (!playerTexture.LoadFromFile(
        "Src/Assets/Textures/player_idle.png"))
    {
        return false;
    }

    if (!tilesetTexture.LoadFromFile(
        "Src/Assets/ObjectSprite/Tileset.png"))
    {
        return false;
    }

    if (!objectsTexture.LoadFromFile(
        "Src/Assets/ObjectSprite/Objects.png"))
    {
        return false;
    }

    if (!spikesTexture.LoadFromFile(
        "Src/Assets/ObjectSprite/Spikes.png"))
    {
        return false;
    }

    if (!predatorTexture.LoadFromFile(
        "Src/Assets/ObjectSprite/Predator_plant.png"))
    {
        return false;
    }

    //--------------------------------------------------
    // World Layout
    //--------------------------------------------------

    const float groundY = 690.0f;

    //--------------------------------------------------
    // Platform
    //--------------------------------------------------

    glm::vec4 platformUV = PixelRegionToUV(
        160.0f,
        20.0f,
        120.0f,
        120.0f,
        static_cast<float>(tilesetTexture.GetWidth()),
        static_cast<float>(tilesetTexture.GetHeight())
    );

    glm::vec2 platformSize(
        180.0f,
        180.0f
    );

    glm::vec2 platformPosition(
        350.0f,
        groundY - platformSize.y
    );

    worldSprites.push_back({
        &tilesetTexture,
        platformPosition,
        platformSize,
        glm::vec2(platformUV.x, platformUV.y),
        glm::vec2(platformUV.z, platformUV.w)
        });

    //--------------------------------------------------
    // Tree
    //--------------------------------------------------

    glm::vec4 treeUV = PixelRegionToUV(
        20.0f,
        10.0f,
        210.0f,
        280.0f,
        static_cast<float>(objectsTexture.GetWidth()),
        static_cast<float>(objectsTexture.GetHeight())
    );

    glm::vec2 treeSize(
        250.0f,
        360.0f
    );

    glm::vec2 treePosition(
        20.0f,
        groundY - treeSize.y
    );

    worldSprites.push_back({
        &objectsTexture,
        treePosition,
        treeSize,
        glm::vec2(treeUV.x, treeUV.y),
        glm::vec2(treeUV.z, treeUV.w)
        });

    //--------------------------------------------------
    // Spikes
    //--------------------------------------------------

    glm::vec2 spikesSize(
        250.0f,
        35.0f
    );

    glm::vec2 spikesPosition(
        550.0f,
        groundY - spikesSize.y
    );

    worldSprites.push_back({
        &spikesTexture,
        spikesPosition,
        spikesSize,
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 1.0f)
        });

    //--------------------------------------------------
    // Predator Plant
    //--------------------------------------------------

    glm::vec4 predatorUV = PixelRegionToUV(
        0.0f,
        0.0f,
        170.0f,
        170.0f,
        static_cast<float>(predatorTexture.GetWidth()),
        static_cast<float>(predatorTexture.GetHeight())
    );

    glm::vec2 predatorSize(
        100.0f,
        100.0f
    );

    glm::vec2 predatorPosition(
        850.0f,
        groundY - predatorSize.y
    );

    worldSprites.push_back({
        &predatorTexture,
        predatorPosition,
        predatorSize,
        glm::vec2(predatorUV.x, predatorUV.y),
        glm::vec2(predatorUV.z, predatorUV.w)
        });
    return true;
}

//--------------------------------------------------
// Input
//--------------------------------------------------

void Game::ProcessInput(float deltaTime)
{
    HandleKeyboardInput(deltaTime);
}

void Game::HandleKeyboardInput(float deltaTime)
{
    player.IsMoving = false;
    player.Velocity.x = 0.0f;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        player.Velocity.x = -player.Speed;
        player.IsMoving = true;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        player.Velocity.x = player.Speed;
        player.IsMoving = true;
    }

    player.Position +=
        player.Velocity * deltaTime;
}

//--------------------------------------------------
// Update
//--------------------------------------------------

void Game::Update(
    float deltaTime)
{
    player.UpdateAnimation(deltaTime);
}

//--------------------------------------------------
// Render
//--------------------------------------------------

void Game::Render()
{
    //--------------------------------------------------
    // Background
    //--------------------------------------------------

    spriteRenderer.DrawSprite(
        backgroundTexture.GetID(),
        glm::vec2(0.0f),
        glm::vec2(
            static_cast<float>(screenWidth),
            static_cast<float>(screenHeight)
        )
    );

    //--------------------------------------------------
    // World objects
    //--------------------------------------------------

    for (const WorldSprite& sprite : worldSprites)
    {
        spriteRenderer.DrawSprite(
            sprite.texture->GetID(),
            sprite.position,
            sprite.size,
            0.0f,
            glm::vec4(1.0f),
            sprite.uvOffset,
            sprite.uvScale
        );
    }

    //--------------------------------------------------
    // Player
    //--------------------------------------------------

    glm::vec2 playerUVScale(
        1.0f / static_cast<float>(player.FrameCount),
        1.0f
    );

    glm::vec2 playerUVOffset(
        static_cast<float>(player.CurrentFrame)
        * playerUVScale.x,
        0.0f
    );

    spriteRenderer.DrawSprite(
        playerTexture.GetID(),
        player.Position,
        player.Size,
        0.0f,
        glm::vec4(1.0f),
        playerUVOffset,
        playerUVScale
    );
}

//--------------------------------------------------
// Shutdown
//--------------------------------------------------

void Game::Shutdown()
{
}

//--------------------------------------------------
// Destructor
//--------------------------------------------------

Game::~Game()
{
}