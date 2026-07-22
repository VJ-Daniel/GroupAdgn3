// Headers/Collision.h
#pragma once

#include <vector>

#include <glm.hpp>

//--------------------------------------------------
// Collision (2D AABB)
//
// Project conventions:
//
//   * position = TOP-LEFT corner
//   * size     = full (width, height)
//   * origin   = top-left of screen,
//                Y pointing DOWN
//
// The player and every solid object are
// boxes, so collision is a box-vs-box
// (AABB) overlap test.
//
// IMPORTANT - sprite quads vs hitboxes:
// A sprite quad is usually much larger
// than the art inside it (transparent
// padding). Collision therefore uses
// SHRUNK boxes built with MakeBox(),
// not the raw sprite rectangles.
//--------------------------------------------------

struct AABB
{
    glm::vec2 position;   // top-left
    glm::vec2 size;       // width, height
};

namespace Collision
{
    //--------------------------------------------------
    // Box Builder
    //
    // Builds a hitbox INSIDE a sprite
    // quad.
    //
    //   spritePos  / spriteSize : the quad
    //   fracX / fracY           : how much
    //       of the quad the hitbox covers
    //       (0..1). 1.0 = whole quad.
    //   anchorBottom : if true, the box is
    //       pinned to the bottom of the
    //       quad (right for things standing
    //       on the ground). Otherwise it is
    //       vertically centred.
    //
    // The box is always centred horizontally.
    //--------------------------------------------------

    AABB MakeBox(
        const glm::vec2& spritePos,
        const glm::vec2& spriteSize,
        float fracX,
        float fracY,
        bool anchorBottom);

    //--------------------------------------------------
    // Overlap Test
    //--------------------------------------------------

    bool Overlaps(
        const AABB& a,
        const AABB& b);

    //--------------------------------------------------
    // Any Solid Hit
    //--------------------------------------------------

    bool HitsAny(
        const AABB& box,
        const std::vector<AABB>& solids);

    //--------------------------------------------------
    // Resolve Movement
    //
    // Returns the furthest safe top-left
    // position for the player's HITBOX.
    //
    // X and Y are resolved separately so
    // the player slides along walls.
    //
    // Anti-stuck guard: if the player is
    // ALREADY overlapping something (bad
    // spawn, obstacle moved onto them),
    // movement is allowed through so they
    // can escape instead of freezing
    // forever.
    //--------------------------------------------------

    glm::vec2 ResolveMovement(
        const glm::vec2& oldPosition,
        const glm::vec2& desiredPosition,
        const glm::vec2& playerSize,
        const std::vector<AABB>& solids);
}
