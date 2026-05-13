#pragma once
#include "Content/Object/ObjectId.h"

struct ProjectileSweepQuery
{
    ObjectId projectileId;
    ObjectId ownerId;
    
    SE::Math::Vector3 from;
    SE::Math::Vector3 to;
    float radius;
    
    bool hitMap = true;             // 지형과의 충돌 검사 여부
    bool hitBlockActor = true;      // 장애물 오브젝트와의 충돌 검사 여부
    bool hitHurtBox = true;         // 피격박스와의 충돌 검사 여부
};
