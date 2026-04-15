#pragma once

struct ActionState
// Action State를 참조하여 충돌 처리를 할 때 사용해야 함
{
    bool isAiming = false;
    bool isJumping = false;
    bool isDoubleJumping = false;
    bool isCrouching = false;
    bool isWireActing = false;
};


