#pragma once

struct CombatState
{
    uint32 weaponId = 0;
    int ammoInMag = 0;
    int reserveAmmo = 0;
    bool isReloading = false;
};

struct ActionState
{
    bool isJumping = false;
    bool isCrouching = false;
    bool isWireActing = false;
};


