#pragma once
#include "Actor.h"
#include "Content/Gameplay/Inventory/ItemTypes.h"

class ObjectManager;
class PlayerPawn;

/*-------------------
   WorldItemActor
-------------------*/
//
// WorldItemActor는 월드에 존재하는 아이템을 나타내는 액터입니다.
// 플레이어가 접근하여 아이템을 획득할 수 있습니다.
// 약간의 탄성을 가지고 떨어집니다.
//

class WorldItemActor : public Actor
{
public:
   WorldItemActor() = default;
   virtual ~WorldItemActor() = default;
   
   WorldItemActor(const WorldItemActor&) = delete;
   WorldItemActor& operator=(const WorldItemActor&) = delete;
   
public:
   virtual se::common::ObjectType GetObjectType() const override { return se::common::OBJ_ITEM; }
   
public:
   const ItemStack& GetItemStack() const { return stack_; }
   void SetItemStack(const ItemStack& stack) { stack_ = stack; }
   
   uint32 GetTemplateId() const { return stack_.id; }
   uint32 GetAmount() const { return stack_.count; }
   
   void InitDrop(const ItemStack& stack, const Vector3& pos, const Vector3& initVel);

protected:
   void OnSpawn() override;
   void Tick(float dt) override;
   
private:
   ItemStack stack_{};
   
};
