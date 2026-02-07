#pragma once
#include "Content/Shared/BaseComponent.h"
#include "LootSourceTypes.h"
#include "LootTableService.h"

class LootTableService;
class ObjectManager;

/*-----------------------
   LootSourceComponent
-----------------------*/
//
// LootSourceComponent는 게임 내에서 아이템을 드롭할 수 있는 객체에 부착되는 컴포넌트입니다.
//

class LootSourceComponent : public BaseComponent
{
public:
   void Init(ObjectId owner, int32 tableId)
   {
      SetOwner(owner);
      
      tableId_ = tableId;
      enabled_ = true;
      generatedOnce_ = false;
   }
   
   void SetEnabled(bool enable) { enabled_ = enable; }
   bool IsEnabled() const { return enabled_; }
   
   void SetTableId(int32 tableId) { tableId_ = tableId; }
   int32 GetTableId() const { return tableId_; }
   
   void SetGenerateOnce(bool generateOnce) { generateOnce_ = generateOnce; }
   bool IsGenerateOnce() const { return generateOnce_; }
   
   bool CanGenerateLoot() const
   {
      if (not enabled_) return false;
      if (tableId_ <= 0) return false;
      if (generateOnce_ and generatedOnce_) return false;   // 이미 한 번 생성된 경우
      
      return true;
   }
   
   // 이 LootSource에서 아이템을 드롭합니다.
   LootSourceResult GenerateLoot(ObjectManager& om, LootTableService& service, const LootSourceContext& ctx)
   {
      LootSourceResult result;
      if (not CanGenerateLoot()) {
         return result;
      }
      
      result.bundle = service.Roll(tableId_, ctx.rngSeed, ctx.roll);
      result.generated = not result.bundle.Empty();
      
      if (generateOnce_)
         generateOnce_ = true;
      
      return result;
   }
   
   // 다시 생성 가능하도록 상태를 초기화합니다.
   void ResetGenerated()
   {
      generatedOnce_ = false;
   }
   
private:
   int32 tableId_{0};
   bool enabled_{true};
   
   bool generateOnce_{true};
   bool generatedOnce_{false};
   
};
