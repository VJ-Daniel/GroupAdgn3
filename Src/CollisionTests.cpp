// CollisionTests.cpp
#include "Headers/CollisionTests.h"

#include "Headers/Collision.h"

#include <iostream>
#include <string>
#include <vector>

#include <glm.hpp>

namespace
{
    int testsRun = 0;
    int testsPassed = 0;

    void Check(
        const std::string& name,
        bool condition)
    {
        testsRun++;

        if (condition)
        {
            testsPassed++;
            std::cout << "[PASS] " << name << std::endl;
        }
        else
        {
            std::cout << "[FAIL] " << name << std::endl;
        }
    }
}

bool RunCollisionSelfTests()
{
    testsRun = 0;
    testsPassed = 0;

    std::cout
        << std::endl
        << "===== COLLISION SELF TESTS ====="
        << std::endl;

    // A 100 x 100 solid at top-left (200, 200).
    //   left=200 right=300 top=200 bottom=300
    std::vector<AABB> solids;
    solids.push_back({ glm::vec2(200.0f, 200.0f), glm::vec2(100.0f, 100.0f) });

    const glm::vec2 playerSize(40.0f, 40.0f);

    // 1. Overlap from each side (player 40 wide, center-ish placement)
    Check("Overlap from left",
        Collision::HitsAny({ { 170.0f, 240.0f }, playerSize }, solids));
    Check("Overlap from right",
        Collision::HitsAny({ { 290.0f, 240.0f }, playerSize }, solids));
    Check("Overlap from top",
        Collision::HitsAny({ { 240.0f, 170.0f }, playerSize }, solids));
    Check("Overlap from bottom",
        Collision::HitsAny({ { 240.0f, 290.0f }, playerSize }, solids));

    // 2. Clear miss from each side
    Check("Miss from left",
        !Collision::HitsAny({ { 150.0f, 240.0f }, playerSize }, solids));
    Check("Miss from right",
        !Collision::HitsAny({ { 310.0f, 240.0f }, playerSize }, solids));
    Check("Miss from top",
        !Collision::HitsAny({ { 240.0f, 150.0f }, playerSize }, solids));
    Check("Miss from bottom",
        !Collision::HitsAny({ { 240.0f, 310.0f }, playerSize }, solids));

    // 3. Inside
    Check("Overlap when player inside",
        Collision::HitsAny({ { 230.0f, 230.0f }, playerSize }, solids));

    // 4. Corner
    Check("Overlap at corner",
        Collision::HitsAny({ { 270.0f, 270.0f }, playerSize }, solids));
    Check("Miss just past corner",
        !Collision::HitsAny({ { 301.0f, 301.0f }, playerSize }, solids));

    // 5. Touching edge is not a hit (right edge = 200 == solid left)
    Check("Touching edge is not a hit",
        !Collision::HitsAny({ { 160.0f, 240.0f }, playerSize }, solids));

    // Movement resolution against a tall wall:
    //   top-left (300,100) size 40x400 -> x 300..340, y 100..500
    std::vector<AABB> wall;
    wall.push_back({ glm::vec2(300.0f, 100.0f), glm::vec2(40.0f, 400.0f) });

    // 6. Head-on X blocked
    {
        glm::vec2 oldPos(250.0f, 200.0f);
        glm::vec2 desired(290.0f, 200.0f);  // right edge -> 330, inside wall
        glm::vec2 r = Collision::ResolveMovement(oldPos, desired, playerSize, wall);
        Check("Head-on X move blocked", r.x == oldPos.x);
    }

    // 7. Sliding: X blocked, Y allowed
    {
        glm::vec2 oldPos(250.0f, 200.0f);
        glm::vec2 desired(290.0f, 160.0f);
        glm::vec2 r = Collision::ResolveMovement(oldPos, desired, playerSize, wall);
        Check("Slide: X blocked", r.x == oldPos.x);
        Check("Slide: Y still moves", r.y == desired.y);
    }

    // 8. Open space: full move allowed
    {
        glm::vec2 oldPos(20.0f, 20.0f);
        glm::vec2 desired(60.0f, 55.0f);
        glm::vec2 r = Collision::ResolveMovement(oldPos, desired, playerSize, wall);
        Check("Open space: full move allowed",
            r.x == desired.x && r.y == desired.y);
    }

    // 9. Cannot land inside a wall
    {
        glm::vec2 oldPos(250.0f, 200.0f);
        glm::vec2 desired(305.0f, 200.0f);
        glm::vec2 r = Collision::ResolveMovement(oldPos, desired, playerSize, wall);
        Check("Cannot land inside the wall", r.x == oldPos.x);
    }


    // 10. Anti-stuck: already overlapping -> movement allowed out
    {
        std::vector<AABB> box;
        box.push_back({ glm::vec2(0.0f, 0.0f), glm::vec2(200.0f, 200.0f) });

        glm::vec2 stuckPos(50.0f, 50.0f);      // inside the box
        glm::vec2 escape(-100.0f, 50.0f);      // heading out

        glm::vec2 r = Collision::ResolveMovement(stuckPos, escape, playerSize, box);
        Check("Anti-stuck: can escape when spawned inside",
            r.x == escape.x && r.y == escape.y);
    }

    // 11. MakeBox shrinks and centres horizontally
    {
        AABB b = Collision::MakeBox(
            glm::vec2(100.0f, 500.0f),
            glm::vec2(200.0f, 128.0f),
            0.5f, 0.5f, true);

        Check("MakeBox: half size",
            b.size.x == 100.0f && b.size.y == 64.0f);
        Check("MakeBox: centred horizontally",
            b.position.x == 150.0f);
        Check("MakeBox: anchored to bottom",
            b.position.y == 564.0f);
    }

    std::cout
        << "================================" << std::endl
        << "Result: " << testsPassed << " / " << testsRun
        << " passed" << std::endl << std::endl;

    return testsPassed == testsRun;
}
