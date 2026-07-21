#pragma once

#include <glm.hpp>

class Player
{
public:
    Player(
        glm::vec2 position,
        glm::vec2 size,
        float speed
    );

    glm::vec2 Position;
    glm::vec2 Size;
    glm::vec2 Velocity;

    float Speed;

    int CurrentFrame;
    int FrameCount;

    float AnimationTimer;
    float AnimationSpeed;

    bool IsMoving;

    void UpdateAnimation(float deltaTime);
};