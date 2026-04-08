#pragma once
#include "Core/Thread/ThreadManager.h"
#include "Service/Room/RoomIdGenerator.h"
#include "Shard/ShardManager.h"

class PlayerSessionLifecycleService;
class ServerPacketDispatcher;
class ServerConfigReader;
class PlayerManager;
class SessionManager;
class RoomDirectory;
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
   TimeThiefServerApp();
   ~TimeThiefServerApp();
   
public:
   bool Init(int argc, char* argv[]);
   bool Start();
   void Run();
   void Shutdown();
   
private:
   bool InitCore();
   bool LoadConfig(int argc, char* argv[]);
   bool CreateManagers();
   bool CreateNetworkService();
   bool CreatePacketDispatcher();
   bool StartServices();
   void LaunchWorkerThreads();
   void LaunchMatchThread();
   
private:
   bool CreateInitialRooms();
   RoomId GenerateRoomId();
   
private:
   void WorkerLoop();
   void MatchLoop();
   
private:
   std::shared_ptr<IocpServerService>              networkService_;
   std::unique_ptr<RoomDirectory>                  roomDirectory_;
   std::unique_ptr<MatchMaker>                     matchMaker_;
   std::unique_ptr<SessionManager>                 sessionManager_;
   std::unique_ptr<ShardManager>                   shardManager_;
   std::unique_ptr<PlayerManager>                  playerManager_;
   std::unique_ptr<ServerConfigReader>             configReader_;
   std::unique_ptr<ServerPacketDispatcher>         packetDispatcher_;
   std::unique_ptr<PlayerSessionLifecycleService>  playerSessionLifecycleService_;
   ThreadManager                                   threadManager_{};
   
   RoomIdGenerator                                 roomIdGenerator_{};
   
   std::atomic<bool>                               running_ { false };
    
};
