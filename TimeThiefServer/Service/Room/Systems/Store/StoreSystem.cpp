#include "pch.h"
#include "StoreSystem.h"
#include "Content/Object/Actor/PlayerPawn.h"
#include "Service/Room/Room.h"

namespace 
{
   constexpr float kStoreInteractionDistance = 300.0f;   // TEMP: s임시 값
   
}

/*----------------
   StoreSystem
----------------*/

bool StoreSystem::Init(Room* ownerRoom)
{
   if (!ownerRoom)
      return false;   // 유효하지 않은 ownerRoom
   
   ownerRoom_ = ownerRoom;
   return true;
}

StoreBuyResult StoreSystem::Buy(const StoreBuyRequest& req)
{
   StoreBuyResult result;
   result.playerId = req.playerId;
   result.storeId = req.storeId;
   result.entryId = req.entryId;
   
   if (!ownerRoom_)
      return result;
   
   // TODO: 구매 로직 구현 (예: 플레이어의 골드 확인, 아이템 재고 확인, 구매 처리 등)
   //       1. 객체 찾기
   //       2. 상점 엔트리 찾기
   //       3. 거리 검사
   //       4. 상태 검사
   //       5. 비용 계산
   //       6. 재화 차감
   //       7. 아이템 지급
   //       8. 결과 반환
   StoreBuyContext ctx{};
   StoreBuyResult validateResult = ValidateBuyRequest(req, ctx);
   if (!validateResult.success)
      return validateResult;
   
   if (!TryConsumeCost(ctx)) {
      result.resultCode = StoreBuyResultCode::NotEnoughCurrency;
      return result;
   }
   
   if (!TryApplyReward(ctx)) {
      result.resultCode = StoreBuyResultCode::ApplyFailed;
      return result;
   }
   
   result.success = true;
   result.resultCode = StoreBuyResultCode::Success;
   return result;
}

StoreBuyResult StoreSystem::ValidateBuyRequest(const StoreBuyRequest& req, StoreBuyContext& ctx)
{
   StoreBuyResult result;
   result.playerId = req.playerId;
   result.storeId = req.storeId;
   result.entryId = req.entryId;
   
   if (!ownerRoom_)
      return result;
   
   auto& om = ownerRoom_->GetObjectManager();
   
   auto* player = ownerRoom_->GetObjectManager().FindAs<PlayerPawn>(req.playerId);
   if (!player) {
      result.resultCode = StoreBuyResultCode::InvalidPlayer;
      return result;
   }
   
   auto* store = ownerRoom_->GetObjectManager().FindAs<Actor>(req.storeId);
   if (!store) {
      result.resultCode = StoreBuyResultCode::InvalidStore;
      return result;
   }
   // TODO: store가 상점이 맞는지 확인하는 로직 필요 (예: 특정 컴포넌트 존재 여부 혹은 EntityType 등)
   
   int64 cost = FindStoreEntry(req.entryId);
   if (cost <= 0) {
      result.resultCode = StoreBuyResultCode::InvalidEntry;
      return result;
   }
   
   if (!CanInteractStore(player, store)) {
      result.resultCode = StoreBuyResultCode::TooFar;
      return result;
   }
   
   if (!CanBuy(player)) {
      result.resultCode = StoreBuyResultCode::InvalidState;
      return result;
   }
   
   // TODO: 구매 제한 체크 (구매 횟수)
   
   ctx.playerPawn = player;
   ctx.storeActor = store;
   ctx.entryId = req.entryId;
   ctx.cost = cost;
   
   result.success = true;
   result.resultCode = StoreBuyResultCode::Success;
   return result;
}

bool StoreSystem::TryConsumeCost(StoreBuyContext& ctx)
{
   if (!ctx.playerPawn)
      return false;
   
   auto& walletComp = ctx.playerPawn->GetWallet();
   MoneyChangeResult result = walletComp.SpendMoney(ownerRoom_->GetObjectManager(), CurrencyType::TimePoint, ctx.cost, MoneyChangeContext{MoneyChangeReason::Purchase, ctx.entryId});
   
   return result.accepted;
}

bool StoreSystem::TryApplyReward(StoreBuyContext& ctx)
{
   if (!ctx.playerPawn)
      return false;
   
   // TODO: 아이템 적용 필요 (아이템은 Inventory 추가, 스킬은 소유, 무기 강화, 스텟 강화 처리 등)
   return false;
}

int64 StoreSystem::FindStoreEntry(uint32 entryId) const
{
   if (entryId == 0)
      return -1;  // 유효하지 않은 entryId
   
   // TODO: Store DataTable에 접근하여 조회
   return -1;
}

bool StoreSystem::CanInteractStore(PlayerPawn* playerPawn, Actor* storeActor) const
{
   if (!playerPawn || !storeActor)
      return false;
   
   const auto& playerPos = playerPawn->GetPosition();
   const auto& storePos = storeActor->GetPosition();
   const float distSq = (playerPos - storePos).LengthSq();
   
   return distSq <= (kStoreInteractionDistance * kStoreInteractionDistance);
}

bool StoreSystem::CanBuy(PlayerPawn* playerPawn) const
{
   if (!playerPawn)
      return false;
   
   if (not playerPawn->IsHpAlive())
      return false;
   
   // 다른 Player 상태 체크도 필요하다면 추가하기
   
   return true;
}
