#include "pch.h"
#include "SessionManager.h"

/*------------------
   SessionManager
------------------*/

SessionManager g_SessionManager;

void SessionManager::Add(SessionRef session)
{
   std::lock_guard<std::mutex> lock(mutex_);
   sessions_.insert(session);
}

void SessionManager::Remove(SessionRef session)
{
   std::lock_guard<std::mutex> lock(mutex_);
   sessions_.erase(session);
}

void SessionManager::Clear()
{
   std::lock_guard<std::mutex> lock(mutex_);
   sessions_.clear();
}

void SessionManager::Broadcast(SendBufferRef sendBuffer)
{
   std::lock_guard<std::mutex> lock(mutex_);
   
   for (const auto& session : sessions_) {
      session->Send(sendBuffer);
   }
}

size_t SessionManager::GetSessionCount() const
{
   return sessions_.size();
}
