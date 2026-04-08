#include "pch.h"
#include "PlayerManager.h"

/*-----------------
   PlayerManager
-----------------*/

PlayerManager::PlayerRef PlayerManager::Create(PlayerId playerId)
{
   // write lock
   std::unique_lock<std::shared_mutex> lock(mutex_);
   
   auto it = playersById_.find(playerId);
   if (it != playersById_.end())
      return it->second;
   
   // TODO: Player 초기화 제대로 하기 (Player 한정자 변경)
   PlayerRef newPlayer = std::make_shared<Player>();
   playersById_.emplace(playerId, newPlayer);
   
   return newPlayer;
}

void PlayerManager::Remove(PlayerId playerId)
{
   // write lock
   std::unique_lock<std::shared_mutex> lock(mutex_);
   
   playersById_.erase(playerId);
}

void PlayerManager::Clear()
{
   // write lock
   std::unique_lock<std::shared_mutex> lock(mutex_);
   
   playersById_.clear();
}

PlayerManager::PlayerRef PlayerManager::Find(PlayerId playerId) const
{
   // read lock
   std::shared_lock<std::shared_mutex> lock(mutex_);
   
   auto it = playersById_.find(playerId);
   if (it == playersById_.end())
      return nullptr;
   
   return it->second;
}

size_t PlayerManager::GetPlayerCount() const
{
   // read lock
   std::shared_lock<std::shared_mutex> lock(mutex_);
   
   return playersById_.size();
}

void PlayerManager::UpdateRoute(PlayerId playerId, ShardId shardId, RoomId roomId)
{
   std::unique_lock<std::shared_mutex> lock(mutex_);
   
   auto it = playersById_.find(playerId);
   if (it == playersById_.end())
      return;   // 플레이어가 존재하지 않음
   
   auto& player = it->second;
   if (!player)
      return;   // 플레이어 참조가 유효하지 않음
   
   player->shardId_ = shardId;
   player->roomId_ = roomId;
}

std::vector<PlayerManager::PlayerRef> PlayerManager::SnapshotPlayers() const
{
   std::vector<PlayerRef> players;
   
   // read lock
   std::shared_lock<std::shared_mutex> lock(mutex_);
   players.reserve(playersById_.size());
   
   for (const auto& [id, player] : playersById_) {
      if (player)
         players.push_back(player);
   }
   
   return players;
}
