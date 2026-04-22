#pragma once
#include "StaticActor.h"

/*--------------
   StoreActor
--------------*/
//
// StoreActor는 상점 역할을 하는 액터입니다.
// 기능은 담지 않고, Store System에서 위치 값 등을 참조하기 위한 용도로만 존재합니다.
//

class StoreActor : public StaticActor
{
public:
    StoreActor() = default;
    virtual ~StoreActor() = default;
    
    StoreActor(const StoreActor& Actor) = delete;
    StoreActor& operator=(const StoreActor& Actor) = delete;
    
public:
    virtual ObjectType GetObjectType() const override { return ObjectType::OBJ_STORE; }
    
protected:
    void OnSpawn() override;
    void OnPreDestroy() override;
    
private:
    
};
