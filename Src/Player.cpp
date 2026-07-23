// player.cpp
#include "Headers/Player.h"

#include <algorithm>

Player::Player(
    glm::vec2 position,
    glm::vec2 size,
    float speed
)
    : Position(position),
    Size(size),
    Velocity(0.0f),
    Speed(speed),
    JumpStrength(800.0f),
    Gravity(2000.0f),
    MaxFallSpeed(1200.0f),
    IsGrounded(false),
    FacingRight(true),
    JustLanded(false),
    IsDashing(false),
    IsAttacking(false),
    IsHurt(false),
    DashTimer(0.0f),
    AttackTimer(0.0f),
    HurtTimer(0.0f),
    State(PlayerState::Idle),
    animationTimer(0.0f),
    frameDuration(0.12f),
    currentFrame(0),
    frameCount(1),
    animationLoops(true),
    animationFinished(false),
    IsMoving(false)
{}

void Player::SetAnimationState(
    PlayerState newState,
    int newFrameCount,
    float newFrameDuration,
    bool shouldLoop)
{
    newFrameCount = std::max(1, newFrameCount);

    if (State == newState &&
        frameCount == newFrameCount &&
        frameDuration == newFrameDuration &&
        animationLoops == shouldLoop)
    {
        return;
    }

    State = newState;
    frameCount = newFrameCount;
    frameDuration = newFrameDuration;
    animationLoops = shouldLoop;

    currentFrame = 0;
    animationTimer = 0.0f;
    animationFinished = frameCount <= 1;
}

void Player::UpdateAnimation(float deltaTime)
{
    if (frameCount <= 1 ||
        frameDuration <= 0.0f ||
        animationFinished)
    {
        return;
    }

    animationTimer += deltaTime;

    while (animationTimer >= frameDuration)
    {
        animationTimer -= frameDuration;

        if (currentFrame < frameCount - 1)
        {
            ++currentFrame;
        }
        else if (animationLoops)
        {
            currentFrame = 0;
        }
        else
        {
            animationFinished = true;
            animationTimer = 0.0f;
            break;
        }
    }
}

bool Player::IsAnimationFinished() const
{
    return animationFinished;
}

void Player::StartDash(float duration)
{
    IsDashing = true;
    DashTimer = std::max(0.0f, duration);
}

void Player::StartAttack(float duration)
{
    IsAttacking = true;
    AttackTimer = std::max(0.0f, duration);
}

void Player::StartHurt(float duration)
{
    IsHurt = true;
    HurtTimer = std::max(0.0f, duration);
}

void Player::UpdateActionTimers(float deltaTime)
{
    if (IsDashing)
    {
        DashTimer -= deltaTime;

        if (DashTimer <= 0.0f)
        {
            IsDashing = false;
            DashTimer = 0.0f;
        }
    }

    if (IsAttacking)
    {
        AttackTimer -= deltaTime;

        if (AttackTimer <= 0.0f)
        {
            IsAttacking = false;
            AttackTimer = 0.0f;
        }
    }

    if (IsHurt)
    {
        HurtTimer -= deltaTime;

        if (HurtTimer <= 0.0f)
        {
            IsHurt = false;
            HurtTimer = 0.0f;
        }
    }
}
