#include "Headers/Player.h"

Player::Player(
    glm::vec2 position,
    glm::vec2 size,
    float speed
)
    : Position(position),
    Size(size),
    Velocity(0.0f),
    Speed(speed),
    CurrentFrame(0),
    FrameCount(4),
    AnimationTimer(0.0f),
    AnimationSpeed(0.12f),
    IsMoving(false)
{}

void Player::UpdateAnimation(float deltaTime)
{
    if (!IsMoving)
    {
        CurrentFrame = 0;
        AnimationTimer = 0.0f;
        return;
    }

    AnimationTimer += deltaTime;

    while (AnimationTimer >= AnimationSpeed)
    {
        AnimationTimer -= AnimationSpeed;

        CurrentFrame =
            (CurrentFrame + 1) % FrameCount;
    }
}