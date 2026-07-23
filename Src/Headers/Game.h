#ifndef GAME_H
#define GAME_H

#include <glew.h>
#include <glfw3.h>
#include <array>
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

struct PlayerAnimationSheet
{
    Texture* texture = nullptr;
    int frameCount = 0;
};

struct PlayerAnimationClip
{
    std::vector<PlayerAnimationSheet> sheets;
    float frameDuration = 0.12f;
    bool loop = true;

    int GetFrameCount() const;
    float GetDuration() const;
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

    Texture playerIdle1Texture;
    Texture playerIdle2Texture;
    Texture playerWalking1Texture;
    Texture playerWalking2Texture;
    Texture playerJumpTexture;
    Texture playerFallTexture;
    Texture playerLanding1Texture;
    Texture playerLanding2Texture;
    Texture playerDashingTexture;
    Texture playerAttack1Texture;
    Texture playerAttack2Texture;
    Texture playerAttack3Texture;
    Texture playerAttack4Texture;
    Texture playerHurt1Texture;
    Texture playerHurt2Texture;

    std::array<PlayerAnimationClip, 8> playerAnimationClips;

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

    // Remembers whether the jump key was held last frame,
    // so holding it down can't trigger repeated jumps.
    bool jumpKeyHeldLastFrame = false;
    bool dashKeyHeldLastFrame = false;
    bool attackKeyHeldLastFrame = false;

    void HandleKeyboardInput(
        float deltaTime);

    // helper function
    std::vector<AABB> BuildSolids() const;

    bool LoadPlayerAnimationTexture(
        Texture& texture,
        const char* filePath);

    void BuildPlayerAnimationClips();

    const PlayerAnimationClip& GetPlayerAnimationClip(
        PlayerState state) const;

    int GetPlayerAnimationFrameCount(
        PlayerState state) const;

    float GetPlayerAnimationDuration(
        PlayerState state) const;

    PlayerState ChoosePlayerAnimationState() const;

    void ApplyPlayerAnimationState(
        float deltaTime);

    const Texture* GetCurrentPlayerTexture(
        glm::vec2& uvOffset,
        glm::vec2& uvScale) const;

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
