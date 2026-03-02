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
   
   for (auto it = sessionIdByPlayerId_.begin(); it != sessionIdByPlayerId_.end(); )
   {
      if (it->second == sessionId)
         it = sessionIdByPlayerId_.erase(it);
      else
         ++it;
   }
}

void SessionManager::Clear()
{
   std::lock_guard<std::mutex> lock(mutex_);
   sessionsById_.clear();
   sessionIdByPlayerId_.clear();
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
   
   auto pit= sessionIdByPlayerId_.find(playerId);
   if (pit != sessionIdByPlayerId_.end()) {
      // 중복 로그인/중복 바인딩 방지
      if (pit->second == sessionId) 
         return true;
      
      return false;
   }
   
   sessionIdByPlayerId_[playerId] = sessionId;
   return true;
}

void SessionManager::UnbindPlayer(PlayerId playerId)
{
   std::lock_guard<std::mutex> lock(mutex_);
   sessionIdByPlayerId_.erase(playerId);
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

