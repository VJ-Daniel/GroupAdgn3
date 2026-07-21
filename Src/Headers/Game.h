#ifndef GAME_H
#define GAME_H

#include <glew.h>
#include <glfw3.h>
#include <vector>

#include "Shader.h"
#include "SpriteRenderer.h"
#include "Texture.h"
#include "Player.h"

struct WorldSprite
{
    Texture* texture;

    glm::vec2 position;
    glm::vec2 size;

    glm::vec2 uvOffset;
    glm::vec2 uvScale;
};

class Game
{
private:

    //--------------------------------------------------
    // Window
    //--------------------------------------------------

    GLFWwindow* window;

    int screenWidth;

    int screenHeight;

    //--------------------------------------------------
    // Systems
    //--------------------------------------------------

    SpriteRenderer spriteRenderer;

    Texture backgroundTexture;

    Texture playerTexture;

    Texture tilesetTexture;     // Platforms
    Texture objectsTexture;     // Trees, chest, bushes, etc.
    Texture spikesTexture;      // Spikes
    Texture predatorTexture;    // Animated plant

    Player player;

    std::vector<WorldSprite> worldSprites;

private:

    //--------------------------------------------------
    // Input
    //--------------------------------------------------

    void HandleKeyboardInput(
        float deltaTime);

public:

    //--------------------------------------------------
    // Constructor
    //--------------------------------------------------

    Game(
        GLFWwindow* gameWindow,
        int width,
        int height);

    //--------------------------------------------------
    // Initialization
    //--------------------------------------------------

    bool Initialize(
        Shader* shader);

    //--------------------------------------------------
    // Main Loop
    //--------------------------------------------------

    void ProcessInput(
        float deltaTime);

    void Update(
        float deltaTime);

    void Render();

    //--------------------------------------------------
    // Shutdown
    //--------------------------------------------------

    void Shutdown();

    //--------------------------------------------------
    // Destructor
    //--------------------------------------------------

    ~Game();
};

#endif