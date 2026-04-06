#pragma once
#include "Core/Thread/ThreadManager.h"

class MatchMaker;

/*----------------------
   TimeThiefServerApp
----------------------*/
//
// TimeThiefServerApp는 TimeThiefServer의 핵심 애플리케이션 클래스입니다.
//


class TimeThiefServerApp
{
public:
   bool Init(int argc, char* argv[]);
   bool Start();
   void Run();
   void Shutdown();
   
private:
   std::shared_ptr<IocpServerService> networkService_;
   // ShardManager... TODO!
   // RoomDirectory... TODO! (Room을 Shard에 등록할 수 있고, RoomId를 통해 ShardId를 찾을 수 있는 형태로)
   std::unique_ptr<MatchMaker> matchMaker_;
   ThreadManager threadManager_;
   
   std::atomic<bool> running_;
    
};
