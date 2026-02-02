#pragma once
#include "Content/Object/ObjectId.h"

struct ObjectId;

/*-----------------
   BaseComponent
-----------------*/
//
// BaseComponent는 모든 컴포넌트의 기본 클래스입니다.
// 가상 소멸자를 가지지 않으며, 단순히 소유자 ObjectId를 관리합니다.
// 따라서 Component 들은 동적 할당을 해선 안됩니다. (다형 삭제가 이루어지지 않음을 보장해야함)
//

class BaseComponent
{
public:
    void SetOwner(ObjectId owner) { owner_ = owner; }
    ObjectId GetOwner() const { return owner_; }
    bool HasOwner() const { return static_cast<bool>(owner_); }
    
private:
    ObjectId owner_{};
    
};
