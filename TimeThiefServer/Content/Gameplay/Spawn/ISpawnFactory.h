#pragma once
#include "SpawnTypes.h"

class ObjectManager;
struct SE::Math::Vector3;

/*-----------------
   ISpawnFactory
-----------------*/
//
// ISpawnFactory는 스폰 요청을 처리하는 팩토리 인터페이스입니다.
//

class ISpawnFactory
{
public:
    using Vector3 = SE::Math::Vector3;
    
public:
    virtual ~ISpawnFactory() = default;
    
    virtual SpawnResult Spawn(ObjectManager& om, const SpawnRequest& req, const Vector3& position, uint64 nowMs) = 0;
    
};
