// Collision.cpp
#include "Headers/Collision.h"

namespace Collision
{
    //--------------------------------------------------
    // Box Builder
    //--------------------------------------------------

    AABB MakeBox(
        const glm::vec2& spritePos,
        const glm::vec2& spriteSize,
        float fracX,
        float fracY,
        bool anchorBottom)
    {
        glm::vec2 boxSize(
            spriteSize.x * fracX,
            spriteSize.y * fracY);

        float offsetX =
            (spriteSize.x - boxSize.x) * 0.5f;

        float offsetY =
            anchorBottom
            ? (spriteSize.y - boxSize.y)
            : (spriteSize.y - boxSize.y) * 0.5f;

        return
        {
            glm::vec2(
                spritePos.x + offsetX,
                spritePos.y + offsetY),
            boxSize
        };
    }

    //--------------------------------------------------
    // Overlap Test
    //--------------------------------------------------

    bool Overlaps(
        const AABB& a,
        const AABB& b)
    {
        bool overlapX =
            a.position.x < b.position.x + b.size.x
            &&
            a.position.x + a.size.x > b.position.x;

        bool overlapY =
            a.position.y < b.position.y + b.size.y
            &&
            a.position.y + a.size.y > b.position.y;

        return overlapX && overlapY;
    }

    //--------------------------------------------------
    // Any Solid Hit
    //--------------------------------------------------

    bool HitsAny(
        const AABB& box,
        const std::vector<AABB>& solids)
    {
        for (const AABB& solid : solids)
        {
            if (Overlaps(box, solid))
            {
                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------
    // Resolve Movement
    //--------------------------------------------------

    glm::vec2 ResolveMovement(
        const glm::vec2& oldPosition,
        const glm::vec2& desiredPosition,
        const glm::vec2& playerSize,
        const std::vector<AABB>& solids)
    {
        //--------------------------------------------------
        // Anti-Stuck Guard
        //
        // If the player is already inside
        // something, blocking would trap
        // them forever. Let the move happen
        // so they can walk back out.
        //--------------------------------------------------

        AABB current{ oldPosition, playerSize };

        if (HitsAny(current, solids))
        {
            return desiredPosition;
        }

        glm::vec2 resolved = oldPosition;

        //--------------------------------------------------
        // Horizontal (X)
        //--------------------------------------------------

        AABB tryX
        {
            glm::vec2(desiredPosition.x, oldPosition.y),
            playerSize
        };

        if (!HitsAny(tryX, solids))
        {
            resolved.x = desiredPosition.x;
        }

        //--------------------------------------------------
        // Vertical (Y)
        //--------------------------------------------------

        AABB tryY
        {
            glm::vec2(resolved.x, desiredPosition.y),
            playerSize
        };

        if (!HitsAny(tryY, solids))
        {
            resolved.y = desiredPosition.y;
        }

        return resolved;
    }
}
