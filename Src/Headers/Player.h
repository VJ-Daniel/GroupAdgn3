#pragma once

#include <glm.hpp>

enum class PlayerState
{
    Idle,
    Running,
    Jumping,
    Falling,
    Landing,
    Dashing,
    Attacking,
    Hurt
};

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

    float JumpStrength = 800.0f;    // upward launch speed of a jump  (px/sec)
    float Gravity = 2000.0f;   // downward acceleration per second (px/sec^2)
    float MaxFallSpeed = 1200.0f;   // terminal velocity when falling  (px/sec)

    bool IsGrounded = false;       // true while standing on the ground
    bool FacingRight = true;        // last  facing direction    

    bool JustLanded = false;
    bool IsDashing = false;
    bool IsAttacking = false;
    bool IsHurt = false;

    float DashTimer = 0.0f;
    float AttackTimer = 0.0f;
    float HurtTimer = 0.0f;

    PlayerState State = PlayerState::Idle;

    float animationTimer;
    float frameDuration;
    int currentFrame;
    int frameCount;

    bool animationLoops = true;
    bool animationFinished = false;

    bool IsMoving;

    void SetAnimationState(
        PlayerState newState,
        int newFrameCount,
        float newFrameDuration,
        bool shouldLoop);

    void UpdateAnimation(float deltaTime);

    bool IsAnimationFinished() const;

    void StartDash(float duration);
    void StartAttack(float duration);
    void StartHurt(float duration);
    void UpdateActionTimers(float deltaTime);
};
