#pragma once
#include "IPlayerSessionLifecycle.h"

namespace se::common
{
   enum ErrorCode : int;
}

class SessionManager;
class PlayerManager;
class ShardManager;
class ServerConfigReader;  // 이거까지 필요한가?

/*---------------------------------
   PlayerSessionLifecycleService
---------------------------------*/
//
// PlayerSessionLifecycleService는 플레이어 세션의 생명주기를 관리하는 서비스입니다.
//

class PlayerSessionLifecycleService : public IPlayerSessionLifecycle
{
public:
   PlayerSessionLifecycleService(SessionManager& sessionManager, PlayerManager& playerManager, ShardManager& shardManager);
   virtual ~PlayerSessionLifecycleService() override = default;
   
public:
   virtual void OnConnected(PlayerSession& session) override;
   virtual void OnDisconnected(PlayerSession& session) override;
    
   virtual bool HandleHandshake(PlayerSession& session, const se::auth::C_HandshakeReq& pkt) override;
   
private:
   void SendHandshakeRes(PlayerSession& session, bool success, se::common::ErrorCode code, const std::string& message);
   
private:
   SessionManager&   sessionManager_;
   PlayerManager&    playerManager_;
   ShardManager&     shardManager_;
    
};
