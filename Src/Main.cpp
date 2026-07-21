/*
    ============================================================
    Elsa's Crystal Quest - OpenGL 2D Platformer

    Description:

    A 2D side-scrolling fantasy platform game in which the player
    controls Elsa, a young knight searching for magical crystals
    scattered throughout an enchanted forest.

    The project demonstrates:

        - OpenGL initialization
        - Window and context creation
        - Shader management
        - Texture and sprite rendering
        - Animated sprites
        - Delta-time-based movement
        - Player input
        - Jump and double-jump mechanics
        - Dash mechanics
        - Collision detection
        - Side-scrolling world rendering

    Technologies:

        - OpenGL 3.3 Core Profile
        - GLFW
        - GLEW
        - GLM
        - stb_image
        - C++

    ============================================================
*/

#include <glew.h>
#include <glfw3.h>

#include <iostream>

#include "Shader.h"
#include "Game.h"

// ------------------------------------------------------------
// Window settings
// ------------------------------------------------------------

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

constexpr const char* WINDOW_TITLE =
"Elsa's Crystal Quest";

// ------------------------------------------------------------
// Called whenever the window is resized.
//
// The OpenGL viewport must be updated so rendering continues
// to use the full drawable area of the window.
// ------------------------------------------------------------

void FramebufferSizeCallback(
    GLFWwindow* window,
    int width,
    int height)
{
    glViewport(
        0,
        0,
        width,
        height);
}

// ------------------------------------------------------------
// Print GLFW errors to the console.
// ------------------------------------------------------------

void GLFWErrorCallback(
    int errorCode,
    const char* description)
{
    std::cerr
        << "GLFW Error "
        << errorCode
        << ": "
        << description
        << std::endl;
}

// ------------------------------------------------------------
// Program entry point
// ------------------------------------------------------------

int main()
{
    // --------------------------------------------------------
    // Set the GLFW error callback before initialization.
    // --------------------------------------------------------

    glfwSetErrorCallback(
        GLFWErrorCallback);

    // --------------------------------------------------------
    // Initialize GLFW.
    //
    // GLFW handles:
    //
    // - Window creation
    // - OpenGL context creation
    // - Keyboard input
    // - Window events
    // --------------------------------------------------------

    if (!glfwInit())
    {
        std::cerr
            << "Failed to initialize GLFW."
            << std::endl;

        return -1;
    }

    // --------------------------------------------------------
    // Request OpenGL 3.3 Core Profile.
    // --------------------------------------------------------

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MAJOR,
        3);

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MINOR,
        3);

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE);

    // --------------------------------------------------------
    // Create the game window.
    // --------------------------------------------------------

    GLFWwindow* window =
        glfwCreateWindow(
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            WINDOW_TITLE,
            nullptr,
            nullptr);

    if (!window)
    {
        std::cerr
            << "Failed to create the GLFW window."
            << std::endl;

        glfwTerminate();

        return -1;
    }

    // --------------------------------------------------------
    // Make the new window's OpenGL context active.
    // --------------------------------------------------------

    glfwMakeContextCurrent(
        window);

    // --------------------------------------------------------
    // Enable vertical synchronization.
    //
    // This usually limits the game to the monitor refresh rate
    // and helps prevent screen tearing.
    // --------------------------------------------------------

    glfwSwapInterval(1);

    // --------------------------------------------------------
    // Register the window resize callback.
    // --------------------------------------------------------

    glfwSetFramebufferSizeCallback(
        window,
        FramebufferSizeCallback);

    // --------------------------------------------------------
    // Initialize GLEW.
    //
    // GLEW loads the modern OpenGL functions required by the
    // game, including shaders, vertex buffers, and textures.
    // --------------------------------------------------------

    glewExperimental = GL_TRUE;

    const GLenum glewResult =
        glewInit();

    if (glewResult != GLEW_OK)
    {
        std::cerr
            << "Failed to initialize GLEW: "
            << glewGetErrorString(
                glewResult)
            << std::endl;

        glfwDestroyWindow(
            window);

        glfwTerminate();

        return -1;
    }

    // --------------------------------------------------------
    // GLEW may generate an unnecessary OpenGL error during
    // initialization. Clear it before continuing.
    // --------------------------------------------------------

    glGetError();

    // --------------------------------------------------------
    // Configure the initial viewport.
    // --------------------------------------------------------

    glViewport(
        0,
        0,
        WINDOW_WIDTH,
        WINDOW_HEIGHT);

    // --------------------------------------------------------
    // Enable alpha blending.
    //
    // This is required for transparent PNG sprites, including
    // Elsa's animation frames and environmental objects.
    // --------------------------------------------------------

    glEnable(
        GL_BLEND);

    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA);

    // --------------------------------------------------------
    // Disable depth testing for the basic 2D renderer.
    //
    // Sprites will be drawn in the order that Render() calls
    // them:
    //
    // 1. Background
    // 2. Platforms and scenery
    // 3. Collectibles and obstacles
    // 4. Player
    // --------------------------------------------------------

    glDisable(
        GL_DEPTH_TEST);

    // --------------------------------------------------------
    // Load the sprite shader.
    //
    // Adjust these paths to match the actual location of your
    // shader files.
    // --------------------------------------------------------

    Shader shader(
        "Src/Assets/Shaders/vertex.glsl",
        "Src/Assets/Shaders/fragment.glsl");

    Shader spriteShader(
        "Src/Assets/Shaders/sprite.vert",
        "Src/Assets/Shaders/sprite.frag");

    // --------------------------------------------------------
    // Create the main Game controller.
    //
    // The Game class will eventually manage:
    //
    // - Player movement
    // - Sprite animation
    // - Texture loading
    // - World rendering
    // - Obstacles
    // - Collision detection
    // - Coins
    // - Crystals
    // - Side-scrolling camera
    // --------------------------------------------------------

    Game game(
        window,
        WINDOW_WIDTH,
        WINDOW_HEIGHT);

    // --------------------------------------------------------
    // Initialize the game systems.
    // --------------------------------------------------------

    if (!game.Initialize(
        &spriteShader))
    {
        std::cerr
            << "Failed to initialize the game."
            << std::endl;

        glfwDestroyWindow(
            window);

        glfwTerminate();

        return -1;
    }

    // --------------------------------------------------------
    // Store the time of the previous frame.
    //
    // Delta time ensures that movement speed does not depend
    // on the computer's frame rate.
    // --------------------------------------------------------

    float lastFrameTime =
        static_cast<float>(
            glfwGetTime());

    // --------------------------------------------------------
    // Main game loop
    // --------------------------------------------------------

    while (!glfwWindowShouldClose(
        window))
    {
        // ----------------------------------------------------
        // Calculate delta time.
        // ----------------------------------------------------

        const float currentFrameTime =
            static_cast<float>(
                glfwGetTime());

        float deltaTime =
            currentFrameTime -
            lastFrameTime;

        lastFrameTime =
            currentFrameTime;

        // ----------------------------------------------------
        // Prevent unusually large frame times.
        //
        // This can happen when debugging or dragging the game
        // window. Without a limit, the player could suddenly
        // move a large distance in one frame.
        // ----------------------------------------------------

        constexpr float MAX_DELTA_TIME =
            0.05f;

        if (deltaTime > MAX_DELTA_TIME)
        {
            deltaTime =
                MAX_DELTA_TIME;
        }

        // ----------------------------------------------------
        // Allow the player to close the application with ESC.
        // ----------------------------------------------------

        if (glfwGetKey(
            window,
            GLFW_KEY_ESCAPE)
            == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(
                window,
                GLFW_TRUE);
        }

        // ----------------------------------------------------
        // Process player input.
        //
        // This will eventually handle:
        //
        // - A / D movement
        // - Left / Right arrow movement
        // - Jump
        // - Double jump
        // - Dash
        // ----------------------------------------------------

        game.ProcessInput(
            deltaTime);

        // ----------------------------------------------------
        // Update the game.
        //
        // This will eventually update:
        //
        // - Player position
        // - Gravity
        // - Jumping
        // - Dash timing
        // - Sprite animations
        // - Collision detection
        // - Camera movement
        // - Coins and crystals
        // ----------------------------------------------------

        game.Update(
            deltaTime);

        // ----------------------------------------------------
        // Render the current frame.
        //
        // The recommended rendering order is:
        //
        // 1. Background
        // 2. World tiles and platforms
        // 3. Obstacles
        // 4. Coins and crystals
        // 5. Enemies
        // 6. Elsa
        // 7. User interface
        // ----------------------------------------------------

        game.Render();

        // ----------------------------------------------------
        // Present the completed frame.
        // ----------------------------------------------------

        glfwSwapBuffers(
            window);

        // ----------------------------------------------------
        // Process keyboard and window events.
        // ----------------------------------------------------

        glfwPollEvents();
    }

    // --------------------------------------------------------
    // Release game resources.
    // --------------------------------------------------------

    game.Shutdown();

    // --------------------------------------------------------
    // Destroy the window and shut down GLFW.
    // --------------------------------------------------------

    glfwDestroyWindow(
        window);

    glfwTerminate();

    return 0;
}