// Game.cpp
#include <iostream>
#include <cstddef>
#include "Game.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>   

namespace
{
    constexpr int PLAYER_FRAME_WIDTH = 100;
    constexpr float DASH_SPEED = 650.0f;

    std::size_t PlayerStateIndex(PlayerState state)
    {
        return static_cast<std::size_t>(state);
    }

    int GetSheetFrameCount(const Texture& texture)
    {
        return texture.GetWidth() / PLAYER_FRAME_WIDTH;
    }
}

int PlayerAnimationClip::GetFrameCount() const
{
    int totalFrames = 0;

    for (const PlayerAnimationSheet& sheet : sheets)
    {
        totalFrames += sheet.frameCount;
    }

    return totalFrames;
}

float PlayerAnimationClip::GetDuration() const
{
    return frameDuration *
        static_cast<float>(GetFrameCount());
}

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
        glm::vec2(100.0f, 594.0f),
        glm::vec2(100.0f, 64.0f),
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

    if (!LoadPlayerAnimationTexture(
        playerIdle1Texture,
        "Src/Assets/PlayerSprite/Idle_KG_1.png") ||
        !LoadPlayerAnimationTexture(
            playerIdle2Texture,
            "Src/Assets/PlayerSprite/Idle_KG_2.png") ||
        !LoadPlayerAnimationTexture(
            playerWalking1Texture,
            "Src/Assets/PlayerSprite/Walking_KG_1.png") ||
        !LoadPlayerAnimationTexture(
            playerWalking2Texture,
            "Src/Assets/PlayerSprite/Walking_KG_2.png") ||
        !LoadPlayerAnimationTexture(
            playerJumpTexture,
            "Src/Assets/PlayerSprite/Jump_KG_1.png") ||
        !LoadPlayerAnimationTexture(
            playerFallTexture,
            "Src/Assets/PlayerSprite/Fall_KG_1.png") ||
        !LoadPlayerAnimationTexture(
            playerLanding1Texture,
            "Src/Assets/PlayerSprite/Landing_KG_1.png") ||
        !LoadPlayerAnimationTexture(
            playerLanding2Texture,
            "Src/Assets/PlayerSprite/Landing_KG_2.png") ||
        !LoadPlayerAnimationTexture(
            playerDashingTexture,
            "Src/Assets/PlayerSprite/Dashing_KG_1.png") ||
        !LoadPlayerAnimationTexture(
            playerAttack1Texture,
            "Src/Assets/PlayerSprite/Attack_KG_1.png") ||
        !LoadPlayerAnimationTexture(
            playerAttack2Texture,
            "Src/Assets/PlayerSprite/Attack_KG_2.png") ||
        !LoadPlayerAnimationTexture(
            playerAttack3Texture,
            "Src/Assets/PlayerSprite/Attack_KG_3.png") ||
        !LoadPlayerAnimationTexture(
            playerAttack4Texture,
            "Src/Assets/PlayerSprite/Attack_KG_4.png") ||
        !LoadPlayerAnimationTexture(
            playerHurt1Texture,
            "Src/Assets/PlayerSprite/Hurt_KG_1.png") ||
        !LoadPlayerAnimationTexture(
            playerHurt2Texture,
            "Src/Assets/PlayerSprite/Hurt_KG_2.png"))
    {
        return false;
    }

    BuildPlayerAnimationClips();

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

    // Shared tree crop has a few px of transparent padding
    // below the visible trunk base, so lift both tree
    // instances slightly off the ground anchor line.
    const float treeGroundOffset = 18.0f;

    //--------------------------------------------------
    // Ground
    //
    // A flat, seamless dirt strip spanning the full
    // screen width so Elsa stands on visible ground
    // instead of floating over empty space between
    // props. Tiled 1:1 at the source tile's native
    // 32x32 size to avoid any stretching artifacts.
    //
    // Four dirt source tiles are cycled (they're subtly
    // different rock/shading variants in the sheet) so
    // the strip doesn't read as one tile repeated.
    //--------------------------------------------------

    const float dirtSourceX[4] = { 160.0f, 192.0f, 224.0f, 256.0f };
    const float groundTileSize = 32.0f;
    const float groundHeight =
        static_cast<float>(screenHeight) - groundY;

    int dirtIndex = 0;

    for (float x = 0.0f;
        x < static_cast<float>(screenWidth);
        x += groundTileSize, ++dirtIndex)
    {
        glm::vec4 dirtUV = PixelRegionToUV(
            dirtSourceX[dirtIndex % 4],
            320.0f,
            32.0f,
            32.0f,
            static_cast<float>(tilesetTexture.GetWidth()),
            static_cast<float>(tilesetTexture.GetHeight())
        );

        groundDirtTiles.push_back({
            &tilesetTexture,
            glm::vec2(x, groundY),
            glm::vec2(groundTileSize, groundHeight),
            glm::vec2(dirtUV.x, dirtUV.y),
            glm::vec2(dirtUV.z, dirtUV.w)
            });
    }

    //--------------------------------------------------
    // Grass cap
    //
    // A row of grass-topped tiles sitting right above
    // the dirt strip, so the ground reads as grass over
    // dirt instead of plain dirt.
    //--------------------------------------------------

    glm::vec4 grassCapUV = PixelRegionToUV(
        224.0f,
        0.0f,
        32.0f,
        32.0f,
        static_cast<float>(tilesetTexture.GetWidth()),
        static_cast<float>(tilesetTexture.GetHeight())
    );

    for (float x = 0.0f;
        x < static_cast<float>(screenWidth);
        x += groundTileSize)
    {
        groundGrassTiles.push_back({
            &tilesetTexture,
            glm::vec2(x, groundY - groundTileSize),
            glm::vec2(groundTileSize, groundTileSize),
            glm::vec2(grassCapUV.x, grassCapUV.y),
            glm::vec2(grassCapUV.z, grassCapUV.w)
            });
    }

    //--------------------------------------------------
    // Platform
    //--------------------------------------------------

    glm::vec4 platformUV = PixelRegionToUV(
        97.0f,
        320.0f,
        58.0f,
        60.0f,
        static_cast<float>(tilesetTexture.GetWidth()),
        static_cast<float>(tilesetTexture.GetHeight())
    );

    glm::vec2 platformSize(
        96.0f,
        96.0f
    );

    glm::vec2 platformPosition(
        650.0f,
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
    //
    // Reuses the same source crop as the second tree
    // further down the path, so both trees match.
    //--------------------------------------------------

    glm::vec4 treeUV = PixelRegionToUV(
        558.0f,
        158.0f,
        100.0f,
        140.0f,
        static_cast<float>(objectsTexture.GetWidth()),
        static_cast<float>(objectsTexture.GetHeight())
    );

    glm::vec2 treeSize(
        160.0f,
        224.0f
    );

    glm::vec2 treePosition(
        20.0f,
        groundY - treeSize.y - treeGroundOffset
    );

    worldSprites.push_back({
        &objectsTexture,
        treePosition,
        treeSize,
        glm::vec2(treeUV.x, treeUV.y),
        glm::vec2(treeUV.z, treeUV.w),
        true    // isTree - draw behind the grass cap
        });

    //--------------------------------------------------
    // Spikes
    //--------------------------------------------------

    glm::vec2 spikesSize(
        144.0f,
        24.0f
    );

    glm::vec2 spikesPosition(
        330.0f,
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
    //
    // The source sheet is a 2x9 animation grid with a
    // lot of transparent padding around each frame; this
    // crop is trimmed to frame 0's actual art so the
    // plant sits flush on the ground instead of floating
    // above it.
    //--------------------------------------------------

    glm::vec4 predatorUV = PixelRegionToUV(
        18.0f,
        33.0f,
        66.0f,
        88.0f,
        static_cast<float>(predatorTexture.GetWidth()),
        static_cast<float>(predatorTexture.GetHeight())
    );

    glm::vec2 predatorSize(
        70.0f,
        94.0f
    );

    glm::vec2 predatorPosition(
        990.0f,
        groundY - predatorSize.y
    );

    worldSprites.push_back({
        &predatorTexture,
        predatorPosition,
        predatorSize,
        glm::vec2(predatorUV.x, predatorUV.y),
        glm::vec2(predatorUV.z, predatorUV.w)
        });

    //--------------------------------------------------
    // Bush
    //
    // Low ground clutter placed between the platform
    // and the predator plant.
    //--------------------------------------------------

    glm::vec4 bushUV = PixelRegionToUV(
        678.0f,
        256.0f,
        54.0f,
        50.0f,
        static_cast<float>(objectsTexture.GetWidth()),
        static_cast<float>(objectsTexture.GetHeight())
    );

    glm::vec2 bushSize(
        56.0f,
        52.0f
    );

    glm::vec2 bushPosition(
        860.0f,
        groundY - bushSize.y
    );

    worldSprites.push_back({
        &objectsTexture,
        bushPosition,
        bushSize,
        glm::vec2(bushUV.x, bushUV.y),
        glm::vec2(bushUV.z, bushUV.w)
        });

    //--------------------------------------------------
    // Second Tree
    //
    // Same source crop as the starting tree, placed
    // further along Elsa's path for a consistent look.
    //--------------------------------------------------

    glm::vec4 tree2UV = PixelRegionToUV(
        558.0f,
        158.0f,
        100.0f,
        140.0f,
        static_cast<float>(objectsTexture.GetWidth()),
        static_cast<float>(objectsTexture.GetHeight())
    );

    glm::vec2 tree2Size(
        120.0f,
        168.0f
    );

    glm::vec2 tree2Position(
        1090.0f,
        groundY - tree2Size.y - treeGroundOffset
    );

    worldSprites.push_back({
        &objectsTexture,
        tree2Position,
        tree2Size,
        glm::vec2(tree2UV.x, tree2UV.y),
        glm::vec2(tree2UV.z, tree2UV.w),
        true    // isTree - draw behind the grass cap
        });

    return true;
}

bool Game::LoadPlayerAnimationTexture(
    Texture& texture,
    const char* filePath)
{
    if (!texture.LoadFromFile(filePath))
    {
        return false;
    }

    if (texture.GetWidth() < PLAYER_FRAME_WIDTH ||
        texture.GetWidth() % PLAYER_FRAME_WIDTH != 0)
    {
        std::cerr
            << "Player animation sheet is not a row of "
            << PLAYER_FRAME_WIDTH
            << "px-wide frames: "
            << filePath
            << " ("
            << texture.GetWidth()
            << " x "
            << texture.GetHeight()
            << ")\n";

        return false;
    }

    return true;
}

void Game::BuildPlayerAnimationClips()
{
    for (PlayerAnimationClip& clip : playerAnimationClips)
    {
        clip = PlayerAnimationClip();
    }

    auto addSheet =
        [](PlayerAnimationClip& clip, Texture& texture)
        {
            clip.sheets.push_back({
                &texture,
                GetSheetFrameCount(texture)
                });
        };

    PlayerAnimationClip& idle =
        playerAnimationClips[PlayerStateIndex(PlayerState::Idle)];
    idle.frameDuration = 0.16f;
    idle.loop = true;
    addSheet(idle, playerIdle1Texture);
    addSheet(idle, playerIdle2Texture);

    PlayerAnimationClip& running =
        playerAnimationClips[PlayerStateIndex(PlayerState::Running)];
    running.frameDuration = 0.08f;
    running.loop = true;
    addSheet(running, playerWalking1Texture);
    addSheet(running, playerWalking2Texture);

    PlayerAnimationClip& jumping =
        playerAnimationClips[PlayerStateIndex(PlayerState::Jumping)];
    jumping.frameDuration = 0.07f;
    jumping.loop = false;
    addSheet(jumping, playerJumpTexture);

    PlayerAnimationClip& falling =
        playerAnimationClips[PlayerStateIndex(PlayerState::Falling)];
    falling.frameDuration = 0.10f;
    falling.loop = true;
    addSheet(falling, playerFallTexture);

    PlayerAnimationClip& landing =
        playerAnimationClips[PlayerStateIndex(PlayerState::Landing)];
    landing.frameDuration = 0.06f;
    landing.loop = false;
    addSheet(landing, playerLanding1Texture);
    addSheet(landing, playerLanding2Texture);

    PlayerAnimationClip& dashing =
        playerAnimationClips[PlayerStateIndex(PlayerState::Dashing)];
    dashing.frameDuration = 0.05f;
    dashing.loop = false;
    addSheet(dashing, playerDashingTexture);

    PlayerAnimationClip& attacking =
        playerAnimationClips[PlayerStateIndex(PlayerState::Attacking)];
    attacking.frameDuration = 0.05f;
    attacking.loop = false;
    addSheet(attacking, playerAttack1Texture);
    addSheet(attacking, playerAttack2Texture);
    addSheet(attacking, playerAttack3Texture);
    addSheet(attacking, playerAttack4Texture);

    PlayerAnimationClip& hurt =
        playerAnimationClips[PlayerStateIndex(PlayerState::Hurt)];
    hurt.frameDuration = 0.08f;
    hurt.loop = false;
    addSheet(hurt, playerHurt1Texture);
    addSheet(hurt, playerHurt2Texture);
}

const PlayerAnimationClip& Game::GetPlayerAnimationClip(
    PlayerState state) const
{
    return playerAnimationClips[PlayerStateIndex(state)];
}

int Game::GetPlayerAnimationFrameCount(
    PlayerState state) const
{
    return GetPlayerAnimationClip(state).GetFrameCount();
}

float Game::GetPlayerAnimationDuration(
    PlayerState state) const
{
    return GetPlayerAnimationClip(state).GetDuration();
}

PlayerState Game::ChoosePlayerAnimationState() const
{
    if (player.IsHurt)
    {
        return PlayerState::Hurt;
    }

    if (player.IsAttacking)
    {
        return PlayerState::Attacking;
    }

    if (player.IsDashing)
    {
        return PlayerState::Dashing;
    }

    if (player.State == PlayerState::Landing &&
        !player.IsAnimationFinished())
    {
        return PlayerState::Landing;
    }

    if (player.JustLanded)
    {
        return PlayerState::Landing;
    }

    if (!player.IsGrounded && player.Velocity.y < -1.0f)
    {
        return PlayerState::Jumping;
    }

    if (!player.IsGrounded && player.Velocity.y > 1.0f)
    {
        return PlayerState::Falling;
    }

    if (player.IsGrounded && player.IsMoving)
    {
        return PlayerState::Running;
    }

    return PlayerState::Idle;
}

void Game::ApplyPlayerAnimationState(
    float deltaTime)
{
    const PlayerState nextState =
        ChoosePlayerAnimationState();

    const PlayerAnimationClip& clip =
        GetPlayerAnimationClip(nextState);

    player.SetAnimationState(
        nextState,
        clip.GetFrameCount(),
        clip.frameDuration,
        clip.loop);

    player.UpdateAnimation(deltaTime);
}

const Texture* Game::GetCurrentPlayerTexture(
    glm::vec2& uvOffset,
    glm::vec2& uvScale) const
{
    const PlayerAnimationClip& clip =
        GetPlayerAnimationClip(player.State);

    int frameIndex = player.currentFrame;

    for (const PlayerAnimationSheet& sheet : clip.sheets)
    {
        if (sheet.texture == nullptr ||
            sheet.frameCount <= 0)
        {
            continue;
        }

        if (frameIndex < sheet.frameCount)
        {
            const float frameWidthUV =
                1.0f / static_cast<float>(sheet.frameCount);

            if (player.FacingRight)
            {
                uvOffset = glm::vec2(
                    static_cast<float>(frameIndex) * frameWidthUV,
                    0.0f);

                uvScale = glm::vec2(
                    frameWidthUV,
                    1.0f);
            }
            else
            {
                uvOffset = glm::vec2(
                    static_cast<float>(frameIndex + 1) * frameWidthUV,
                    0.0f);

                uvScale = glm::vec2(
                    -frameWidthUV,
                    1.0f);
            }

            return sheet.texture;
        }

        frameIndex -= sheet.frameCount;
    }

    uvOffset = glm::vec2(0.0f);
    uvScale = glm::vec2(
        player.FacingRight ? 1.0f : -1.0f,
        1.0f);

    return nullptr;
}

//--------------------------------------------------
// Input
//--------------------------------------------------

void Game::ProcessInput(float deltaTime)
{
    HandleKeyboardInput(deltaTime);
}

std::vector<AABB> Game::BuildSolids() const
{
    std::vector<AABB> solids;
    solids.reserve(worldSprites.size());

    //--------------------------------------------------
    // Sprite quads are much larger than the art inside
    // them (transparent padding), so each solid uses a
    // shrunk hitbox anchored to the bottom of the quad.
    //
    // worldSprites order (from Initialize):
    //   0 platform, 1 tree, 2 spikes, 3 predator plant,
    //   4 bush, 5 second tree
    //--------------------------------------------------

    for (std::size_t i = 0; i < worldSprites.size(); ++i)
    {
        const WorldSprite& s = worldSprites[i];

        float fracX = 0.80f;
        float fracY = 0.60f;

        switch (i)
        {
        case 0:  // platform - wide and solid
            fracX = 0.90f;
            fracY = 0.50f;
            break;

        case 1:  // tree - only the trunk should block
            fracX = 0.20f;
            fracY = 0.55f;
            break;

        case 2:  // spikes - low and wide
            fracX = 0.95f;
            fracY = 0.90f;
            break;

        case 3:  // predator plant - narrow
            fracX = 0.50f;
            fracY = 0.80f;
            break;

        case 4:  // bush - low and round
            fracX = 0.70f;
            fracY = 0.55f;
            break;

        case 5:  // second tree - only the trunk should block
            fracX = 0.20f;
            fracY = 0.55f;
            break;

        default:
            break;
        }

        solids.push_back(
            Collision::MakeBox(
                s.position,
                s.size,
                fracX,
                fracY,
                true));   // anchor to bottom
    }

    return solids;
}

void Game::HandleKeyboardInput(float deltaTime)
{
    // Input only SETS Elsa's velocity / requests a jump.
    // The position update, gravity and collision all happen
    // in Update(), which is why deltaTime is unused here.
    (void)deltaTime;

    //--------------------------------------------------
    // Horizontal movement (A / D or Left / Right arrows)
    //--------------------------------------------------

    bool moveLeft =
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;

    bool moveRight =
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;

    player.Velocity.x = 0.0f;
    player.IsMoving = false;

    if (moveLeft)
    {
        player.Velocity.x = -player.Speed;
        player.IsMoving = true;
        player.FacingRight = false;
    }

    if (moveRight)
    {
        player.Velocity.x = player.Speed;
        player.IsMoving = true;
        player.FacingRight = true;
    }

    //--------------------------------------------------
    // Jump (Space)
    //
    // Edge-triggered: jump only on the frame the key goes
    // from up to down, and only while grounded. This stops
    // a held key or a mid-air tap from re-jumping.
    //--------------------------------------------------

    bool jumpKeyDown =
        glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    bool jumpPressedThisFrame =
        jumpKeyDown && !jumpKeyHeldLastFrame;

    jumpKeyHeldLastFrame = jumpKeyDown;

    if (jumpPressedThisFrame && player.IsGrounded)
    {
        player.Velocity.y = -player.JumpStrength;
        player.IsGrounded = false;
    }

    //--------------------------------------------------
    // Dash (Left Shift / Right Shift)
    //--------------------------------------------------

    bool dashKeyDown =
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    bool dashPressedThisFrame =
        dashKeyDown && !dashKeyHeldLastFrame;

    dashKeyHeldLastFrame = dashKeyDown;

    if (dashPressedThisFrame && !player.IsDashing)
    {
        player.StartDash(
            GetPlayerAnimationDuration(PlayerState::Dashing));
    }

    if (player.IsDashing)
    {
        const float dashDirection =
            player.FacingRight ? 1.0f : -1.0f;

        player.Velocity.x =
            dashDirection * DASH_SPEED;

        player.IsMoving = true;
    }

    //--------------------------------------------------
    // Attack (J)
    //--------------------------------------------------

    bool attackKeyDown =
        glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;

    bool attackPressedThisFrame =
        attackKeyDown && !attackKeyHeldLastFrame;

    attackKeyHeldLastFrame = attackKeyDown;

    if (attackPressedThisFrame && !player.IsAttacking)
    {
        player.StartAttack(
            GetPlayerAnimationDuration(PlayerState::Attacking));
    }
}

//--------------------------------------------------
// Update
//--------------------------------------------------

void Game::Update(
    float deltaTime)
{
    const bool wasGrounded =
        player.IsGrounded;

    //--------------------------------------------------
    // 1. Gravity
    //
    // Each frame gravity increases Elsa's downward
    // (positive-Y) velocity. We cap it at MaxFallSpeed so
    // she can't accelerate without limit.
    //--------------------------------------------------

    player.Velocity.y += player.Gravity * deltaTime;

    if (player.Velocity.y > player.MaxFallSpeed)
    {
        player.Velocity.y = player.MaxFallSpeed;
    }

    //--------------------------------------------------
    // 2. Horizontal movement + wall collision
    //
    // Move on X only, then resolve against the solids so
    // Elsa can't walk through the trees or the rock block.
    // (Vertical is handled separately below so she slides
    // along walls instead of being snapped upward.)
    //--------------------------------------------------

    std::vector<AABB> solids = BuildSolids();

    const float PLAYER_FRAC_X = 0.22f;
    const float PLAYER_FRAC_Y = 0.85f;

    glm::vec2 desiredX(
        player.Position.x + player.Velocity.x * deltaTime,
        player.Position.y);

    AABB oldBox = Collision::MakeBox(
        player.Position, player.Size,
        PLAYER_FRAC_X, PLAYER_FRAC_Y, true);

    AABB desiredBoxX = Collision::MakeBox(
        desiredX, player.Size,
        PLAYER_FRAC_X, PLAYER_FRAC_Y, true);

    glm::vec2 offset = oldBox.position - player.Position;

    glm::vec2 resolvedX = Collision::ResolveMovement(
        oldBox.position,
        desiredBoxX.position,
        oldBox.size,
        solids);

    player.Position = resolvedX - offset;

    //--------------------------------------------------
    // 3. Vertical movement + landing on the ground
    //
    // Apply the (gravity-affected) vertical velocity, then
    // test Elsa's feet against the ground surface. If her
    // feet reach it, snap them onto it, stop the fall, and
    // mark her grounded.
    //
    // NOTE (Phase 3): this uses one flat ground line. In
    // Phase 4 we replace it with real per-object collision
    // so she can also stand ON the platforms.
    //--------------------------------------------------

    const float groundLevelY = 658.0f;   // top of the grass strip

    player.Position.y += player.Velocity.y * deltaTime;

    float feetY = player.Position.y + player.Size.y;

    if (feetY >= groundLevelY)
    {
        player.Position.y = groundLevelY - player.Size.y;
        player.Velocity.y = 0.0f;
        player.IsGrounded = true;
    }
    else
    {
        player.IsGrounded = false;
    }

    player.JustLanded =
        !wasGrounded && player.IsGrounded;

    //--------------------------------------------------
    // 4. Animation
    //--------------------------------------------------

    ApplyPlayerAnimationState(deltaTime);
    player.UpdateActionTimers(deltaTime);
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
    // Draw order (no depth testing - painter's algorithm):
    //
    //   1. Dirt
    //   2. Trees            (so trunk bases sit UNDER the
    //                        grass cap, hiding the roots)
    //   3. Grass cap
    //   4. Remaining props
    //   5. Player
    //--------------------------------------------------

    auto drawWorldSprite = [this](const WorldSprite& sprite)
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
        };

    for (const WorldSprite& tile : groundDirtTiles)
    {
        drawWorldSprite(tile);
    }

    for (const WorldSprite& sprite : worldSprites)
    {
        if (sprite.isTree)
        {
            drawWorldSprite(sprite);
        }
    }

    for (const WorldSprite& tile : groundGrassTiles)
    {
        drawWorldSprite(tile);
    }

    for (const WorldSprite& sprite : worldSprites)
    {
        if (!sprite.isTree)
        {
            drawWorldSprite(sprite);
        }
    }

    //--------------------------------------------------
    // Player
    //--------------------------------------------------

    glm::vec2 playerUVOffset(0.0f);
    glm::vec2 playerUVScale(1.0f);

    const Texture* currentPlayerTexture =
        GetCurrentPlayerTexture(
            playerUVOffset,
            playerUVScale);

    if (currentPlayerTexture != nullptr)
    {
        spriteRenderer.DrawSprite(
            currentPlayerTexture->GetID(),
            player.Position,
            player.Size,
            0.0f,
            glm::vec4(1.0f),
            playerUVOffset,
            playerUVScale
        );
    }
}

//--------------------------------------------------
// Shutdown
//--------------------------------------------------

void Game::Shutdown()
{}

//--------------------------------------------------
// Destructor
//--------------------------------------------------

Game::~Game()
{}
