#ifndef GAME_H
#define GAME_H

#include <glew.h>
#include <glfw3.h>
#include <vector>

#include "Shader.h"
#include "SpriteRenderer.h"
#include "Texture.h"
#include "Player.h"

#include "Collision.h"
struct WorldSprite
{
    Texture* texture;

    glm::vec2 position;
    glm::vec2 size;

    glm::vec2 uvOffset;
    glm::vec2 uvScale;

    // Trees draw before the grass cap so their trunk
    // base/roots sit behind it instead of on top.
    bool isTree = false;
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

    //--------------------------------------------------
    // Ground
    //
    // Separate, background-layer strips of tiles kept
    // out of worldSprites/BuildSolids so they can't shift
    // the index-based hitbox tuning there.
    //
    // Drawn in two passes (dirt, then grass) so trees can
    // be drawn in between - trunk bases sit under the
    // grass cap instead of on top of it.
    //--------------------------------------------------

    std::vector<WorldSprite> groundDirtTiles;
    std::vector<WorldSprite> groundGrassTiles;

private:

    //--------------------------------------------------
    // Input
    //--------------------------------------------------

    void HandleKeyboardInput(
        float deltaTime);
    // helper function
    std::vector<AABB> BuildSolids() const;
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