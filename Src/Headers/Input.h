#pragma once

#include <vector>

#include <glm.hpp>

struct GLFWwindow;
struct AABB;
class Player;

//--------------------------------------------------
// Input
//
// Reads raw keyboard state and turns it into the
// player's per-frame movement:
//
//   A / D          - walk left / right
//   Left / Right   - walk left / right (arrow key alt)
//   Space          - jump (press again in the air
//                    for a double jump)
//   Left Shift     - dash (short horizontal burst,
//                    used to clear wide gaps or
//                    smash through weak obstacles)
//
// Scope on purpose:
//
//   Input only decides WHAT the player wants to do
//   this frame - it writes the result into
//   player.Velocity and player.IsMoving. It does NOT
//   move the player and does NOT touch the solids
//   list to resolve movement; Game::HandleKeyboardInput
//   still owns that step exactly as it did before
//   (Collision::ResolveMovement, BuildSolids(), etc.).
//
//   The one exception is IsGrounded(): Input needs to
//   know whether the player is currently standing on
//   something so it can reset the double-jump counter,
//   so it takes the player's hitbox + the solids list
//   as read-only input for that single check.
//--------------------------------------------------

class Input
{
public:

    Input();

    //--------------------------------------------------
    // Update
    //
    // Call once per frame, BEFORE moving the player.
    //
    //   window       - used to poll key state
    //   deltaTime    - frame time
    //   player       - Velocity / IsMoving are written
    //   playerHitbox - the player's CURRENT (pre-move)
    //                  collision box, used only for the
    //                  grounded check
    //   solids       - current frame's solids, used
    //                  only for the grounded check
    //--------------------------------------------------

    void Update(
        GLFWwindow* window,
        float deltaTime,
        Player& player,
        const AABB& playerHitbox,
        const std::vector<AABB>& solids);

    //--------------------------------------------------
    // IsDashing
    //
    // True for the duration of an active dash. Game.cpp
    // reads this so it can let the player punch straight
    // through solids instead of being blocked like a
    // normal walk - that's the "smash through weak
    // obstacles" part of the dash.
    //--------------------------------------------------

    bool IsDashing() const;

private:

    //--------------------------------------------------
    // Tuning
    //--------------------------------------------------

    static constexpr float GRAVITY = 1800.0f;
    static constexpr float MAX_FALL_SPEED = 1000.0f;
    static constexpr float JUMP_VELOCITY = -650.0f;
    static constexpr int   MAX_JUMPS = 2;   // 1 = normal jump, 2 = double jump

    static constexpr float DASH_SPEED = 900.0f;
    static constexpr float DASH_DURATION = 0.18f;
    static constexpr float DASH_COOLDOWN = 0.6f;

    //--------------------------------------------------
    // Jump state
    //--------------------------------------------------

    int  jumpsUsed;
    bool jumpKeyWasDown;   // edge-detect so holding Space doesn't spam jumps

    //--------------------------------------------------
    // Dash state
    //--------------------------------------------------

    bool  isDashing;
    float dashTimer;
    float dashCooldownTimer;
    float dashDirectionX;   // -1.0f left, +1.0f right
    float lastFacingX;      // remembered facing, so dashing while
    // standing still still has a direction
    bool  dashKeyWasDown;   // edge-detect, one dash per key press

    //--------------------------------------------------
    // Helpers
    //--------------------------------------------------

    bool IsGrounded(
        const AABB& playerHitbox,
        const std::vector<AABB>& solids) const;
};