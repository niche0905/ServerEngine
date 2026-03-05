#include "pch.h"
#include "SessionManager.h"

/*------------------
   SessionManager
------------------*/

void SessionManager::Add(SessionId sessionId, const SessionRef& session)
{
   if (not session) return;
   
   std::lock_guard<std::mutex> lock(mutex_);
   
   sessionsById_[sessionId] = session;
}

void SessionManager::RemoveBySessionId(SessionId sessionId)
{
   std::lock_guard<std::mutex> lock(mutex_);
   
   auto it = sessionsById_.find(sessionId);
   if (it == sessionsById_.end()) return;
   
   sessionsById_.erase(it);
   
   auto rit = playerIdBySessionId_.find(sessionId);
   if (rit != playerIdBySessionId_.end()) {
      const PlayerId playerId = rit->second;
      
      // 양쪽 맵에서 모두 제거
      playerIdBySessionId_.erase(rit);
      sessionIdByPlayerId_.erase(playerId);
   }
}

void SessionManager::Clear()
{
   std::lock_guard<std::mutex> lock(mutex_);
   sessionsById_.clear();
   sessionIdByPlayerId_.clear();
   playerIdBySessionId_.clear();
}

SessionManager::SessionRef SessionManager::FindBySessionId(SessionId sessionId) const
{
   std::lock_guard<std::mutex> lock(mutex_);
   
   auto it = sessionsById_.find(sessionId);
   if (it == sessionsById_.end()) return nullptr;
   
   return it->second;
}

SessionManager::SessionRef SessionManager::FindByPlayerId(PlayerId playerId) const
{
   std::lock_guard<std::mutex> lock(mutex_);
   
   auto pit= sessionIdByPlayerId_.find(playerId);
   if (pit == sessionIdByPlayerId_.end()) return nullptr;
   
   const SessionId sessionId = pit->second;
   
   auto sit = sessionsById_.find(sessionId);
   if (sit == sessionsById_.end()) return nullptr;
   
   return sit->second;
}

bool SessionManager::BindPlayer(SessionId sessionId, PlayerId playerId)
{
   std::lock_guard<std::mutex> lock(mutex_);
   
   if (sessionsById_.find(sessionId) == sessionsById_.end()) return false;
   
   auto p2s = sessionIdByPlayerId_.find(playerId);
   if (p2s != sessionIdByPlayerId_.end()) {
      // 이미 바인딩된 플레이어 ID가 존재하는 경우
      
      if (p2s->second == sessionId) // 같은 세션에 재 바인딩이면 OK
         return true;
      
      return false;  // 정책: 중복 로그인/바인딩 허용 안 함
   }
   
   auto s2p = playerIdBySessionId_.find(sessionId);
   if (s2p != playerIdBySessionId_.end()) {
      // 이미 바인딩된 세션 ID가 존재하는 경우
      
      if (s2p->second == playerId) // 같은 플레이어에 재 바인딩이면 OK
         return true;
      
      return false;  // 정책: 중복 로그인/바인딩 허용 안 함
   }
   
   // 정상 바인딩 처리
   // 양쪽 맵에 바인딩 정보 추가
   sessionIdByPlayerId_[playerId] = sessionId;
   playerIdBySessionId_[sessionId] = playerId;
}

void SessionManager::UnbindPlayer(PlayerId playerId)
{
   std::lock_guard<std::mutex> lock(mutex_);
   
   auto it = sessionIdByPlayerId_.find(playerId);
   if (it == sessionIdByPlayerId_.end()) return;
   
   const SessionId sessionId = it->second;
   
   sessionIdByPlayerId_.erase(it);
   playerIdBySessionId_.erase(sessionId);
}

bool SessionManager::TryGetSessionId(PlayerId plaeyrId, SessionId& outSessionId) const
{
   std::lock_guard<std::mutex> lock(mutex_);
   
   auto it = sessionIdByPlayerId_.find(plaeyrId);
   if (it == sessionIdByPlayerId_.end()) return false;
   
   outSessionId = it->second;
   return true;
}

bool SessionManager::TryGetPlayerId(SessionId sessionId, PlayerId& outPlayerId) const
{
   std::lock_guard<std::mutex> lock(mutex_);
   
   auto it = playerIdBySessionId_.find(sessionId);
   if (it == playerIdBySessionId_.end()) return false;
   
   outPlayerId = it->second;
   return true;
}

size_t SessionManager::GetSessionCount() const
{
   std::lock_guard<std::mutex> lock(mutex_);
   return sessionsById_.size();
}

size_t SessionManager::GetBoundPlayerCount() const
{
   std::lock_guard<std::mutex> lock(mutex_);
   return sessionIdByPlayerId_.size();
}

void SessionManager::Broadcast(SendBufferRef sendBuffer)
{
   if (not sendBuffer) return;
   
   auto sessions = SnapshotSessions();
   
   for (const auto& session : sessions) {
      if (session)
         session->Send(sendBuffer);
   }
}

std::vector<SessionManager::SessionRef> SessionManager::SnapshotSessions() const
{
   std::lock_guard<std::mutex> lock(mutex_);
   
   std::vector<SessionRef> out;
   out.reserve(sessionsById_.size());
   
   for (const auto& [id, session] : sessionsById_) {
      if (session)
         out.push_back(session);
   }
   
   return out;
}

SessionManager g_SessionManager;

