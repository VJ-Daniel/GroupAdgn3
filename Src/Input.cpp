#include "Headers/Input.h"

#include "Headers/Player.h"
#include "Headers/Collision.h"

#include <glfw3.h>

Input::Input()
    : jumpsUsed(0),
    jumpKeyWasDown(false),
    isDashing(false),
    dashTimer(0.0f),
    dashCooldownTimer(0.0f),
    dashDirectionX(1.0f),
    lastFacingX(1.0f),
    dashKeyWasDown(false)
{
}

//--------------------------------------------------
// IsDashing
//--------------------------------------------------

bool Input::IsDashing() const
{
    return isDashing;
}

//--------------------------------------------------
// Grounded Check
//
// Nudge the player's hitbox down a couple pixels and
// see if that touches a solid. The couple px of slack
// means this still reads "grounded" for a frame even
// if float error keeps the box from being pixel-perfect
// flush with the ground.
//--------------------------------------------------

bool Input::IsGrounded(
    const AABB& playerHitbox,
    const std::vector<AABB>& solids) const
{
    AABB probe = playerHitbox;
    probe.position.y += 2.0f;

    return Collision::HitsAny(probe, solids);
}

//--------------------------------------------------
// Update
//--------------------------------------------------

void Input::Update(
    GLFWwindow* window,
    float deltaTime,
    Player& player,
    const AABB& playerHitbox,
    const std::vector<AABB>& solids)
{
    bool grounded = IsGrounded(playerHitbox, solids);

    // Only refill jumps while moving downward or standing
    // still on the ground - not the instant a jump starts,
    // since the launch frame can still overlap the grounded
    // probe.
    if (grounded && player.Velocity.y >= 0.0f)
    {
        jumpsUsed = 0;
    }

    //--------------------------------------------------
    // Timers tick regardless of input
    //--------------------------------------------------

    if (dashCooldownTimer > 0.0f)
    {
        dashCooldownTimer -= deltaTime;
    }

    if (isDashing)
    {
        dashTimer -= deltaTime;

        if (dashTimer <= 0.0f)
        {
            isDashing = false;
        }
    }

    //--------------------------------------------------
    // Walk (A / D, Left / Right arrows)
    //--------------------------------------------------

    player.IsMoving = false;

    float walkX = 0.0f;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        walkX -= player.Speed;
        lastFacingX = -1.0f;
        player.IsMoving = true;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        walkX += player.Speed;
        lastFacingX = 1.0f;
        player.IsMoving = true;
    }

    //--------------------------------------------------
    // Jump (Space)
    //
    // Edge-triggered off the key press, not the held
    // state, so holding Space doesn't fire a new jump
    // every frame. Works again in the air as long as
    // jumps remain, which is what gives the double jump.
    //--------------------------------------------------

    bool jumpKeyDown =
        glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    if (jumpKeyDown && !jumpKeyWasDown && jumpsUsed < MAX_JUMPS)
    {
        player.Velocity.y = JUMP_VELOCITY;
        jumpsUsed++;
    }

    jumpKeyWasDown = jumpKeyDown;

    //--------------------------------------------------
    // Dash (Left Shift)
    //
    // Edge-triggered, gated by a cooldown so it can't be
    // spammed, fires in whichever direction Elsa is
    // currently facing (or last faced, if standing still).
    //--------------------------------------------------

    bool dashKeyDown =
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    if (dashKeyDown && !dashKeyWasDown &&
        !isDashing && dashCooldownTimer <= 0.0f)
    {
        isDashing = true;
        dashTimer = DASH_DURATION;
        dashCooldownTimer = DASH_COOLDOWN;
        dashDirectionX = lastFacingX;
    }

    dashKeyWasDown = dashKeyDown;

    //--------------------------------------------------
    // Gravity
    //
    // Always applied so jumps arc and falls feel
    // weighted. A dash freezes vertical motion for its
    // short duration so it reads as a flat horizontal
    // punch rather than a diagonal jump.
    //--------------------------------------------------

    if (isDashing)
    {
        player.Velocity.y = 0.0f;
    }
    else
    {
        player.Velocity.y += GRAVITY * deltaTime;

        if (player.Velocity.y > MAX_FALL_SPEED)
        {
            player.Velocity.y = MAX_FALL_SPEED;
        }
    }

    //--------------------------------------------------
    // Final horizontal velocity - dash overrides normal
    // walking input for its short duration.
    //--------------------------------------------------

    if (isDashing)
    {
        player.Velocity.x = DASH_SPEED * dashDirectionX;
        player.IsMoving = true;
    }
    else
    {
        player.Velocity.x = walkX;
    }

    //--------------------------------------------------
    // Facing direction
    //
    // Driven off lastFacingX rather than walkX, so Elsa
    // keeps facing the way she last walked (or dashed)
    // while standing still, instead of snapping back to
    // a default facing the moment she stops moving.
    //--------------------------------------------------

    player.IsFacingLeft = (lastFacingX < 0.0f);
}