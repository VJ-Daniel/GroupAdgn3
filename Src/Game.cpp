// Game.cpp
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

    // Shared tree crop has a few px of transparent padding
    // below the visible trunk base, so lift both tree
    // instances slightly off the ground anchor line.
    const float treeGroundOffset = 28.0f;

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
        180.0f,
        180.0f
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
        250.0f,
        360.0f
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
        250.0f,
        35.0f
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
        90.0f,
        120.0f
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
        90.0f,
        84.0f
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
        180.0f,
        250.0f
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

    glm::vec2 desired =
        player.Position + player.Velocity * deltaTime;

    std::vector<AABB> solids = BuildSolids();

    //--------------------------------------------------
    // Elsa's sprite quad is far wider than her artwork,
    // so collision uses a narrow hitbox inside the quad.
    // We resolve the HITBOX, then convert back to the
    // sprite position by removing the same offset.
    //--------------------------------------------------

    const float PLAYER_FRAC_X = 0.22f;
    const float PLAYER_FRAC_Y = 0.85f;

    AABB oldBox = Collision::MakeBox(
        player.Position, player.Size,
        PLAYER_FRAC_X, PLAYER_FRAC_Y, true);

    AABB desiredBox = Collision::MakeBox(
        desired, player.Size,
        PLAYER_FRAC_X, PLAYER_FRAC_Y, true);

    glm::vec2 offset = oldBox.position - player.Position;

    glm::vec2 resolvedBox = Collision::ResolveMovement(
        oldBox.position,
        desiredBox.position,
        oldBox.size,
        solids);

    player.Position = resolvedBox - offset;
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