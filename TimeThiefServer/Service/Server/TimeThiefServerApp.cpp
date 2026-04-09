#include "pch.h"
#include "TimeThiefServerApp.h"
#include <Generated/ServerPacketHandler.h>
#include "Utils/FilePathHelper.h"
#include "Core/Service/IOCP/IocpServerService.h"
#include "Network/ServerConfigReader.h"
#include "Network/PacketDispatcher/ServerPacketDispatcher.h"
#include "Network/Session/Lifecycle/PlayerSessionLifecycleService.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Service/MatchMaking/MatchMaker.h"
#include "Service/Player/PlayerManager/PlayerManager.h"
#include "Shard/RoomDirectory.h"

/*----------------------
   TimeThiefServerApp
----------------------*/

TimeThiefServerApp::TimeThiefServerApp() = default;
TimeThiefServerApp::~TimeThiefServerApp() = default;

bool TimeThiefServerApp::Init(int argc, char* argv[])
{
   if (not InitCore())
      return false;
   
   if (not LoadConfig(argc, argv))
      return false;
   
   if (not CreateManagers())
      return false;
   
   if (not CreateNetworkService())
      return false;
   
   return true;
}

bool TimeThiefServerApp::Start()
{
   if (running_.load())
      return true;
   
   running_.store(true);
   
   if (not StartServices())
      return false;
   
   LaunchWorkerThreads();
   LaunchMatchThread();
   
   consoleLogger->Log(Color::Green, L"[TTSA] Start Completed\n");
   return true;
}

void TimeThiefServerApp::Run()
{
   consoleLogger->Log(Color::Green, L"[TTSA] Run entered\n");

   while (running_.load()) {
      
      std::this_thread::sleep_for(std::chrono::seconds(1));
   }
   
   consoleLogger->Log(Color::Yellow, L"[TTSA] Run finished\n");
}

void TimeThiefServerApp::Shutdown()
{
   bool expected = true;
   if (!running_.compare_exchange_strong(expected, false)) {
      running_.store(false);
   }
   
   consoleLogger->Log(Color::Yellow, L"[TTSA] Shutdown begin\n");
   
   if (networkService_) {
      networkService_->StopService();
   }
   
   if (shardManager_) {
      shardManager_->Stop();
   }
   
   threadManager_.Join();
   
   consoleLogger->Log(Color::Yellow, L"[TTSA] Shutdown end\n");
}


bool TimeThiefServerApp::InitCore()
{
   SE::Init();
   
   consoleLogger->Log(Color::Green, L"[TTSA] Core Initialized\n");
   return true;
}

bool TimeThiefServerApp::LoadConfig(int argc, char* argv[])
{
   configReader_ = std::make_unique<ServerConfigReader>();
   
   auto configPath = ResolveConfigPath(argc, argv);
   bool ok = configReader_->LoadFromFile(configPath.string());
   if (not ok) {
      consoleLogger->Log(Color::Red, L"[TTSA] config load fail\n");
      return false;
   }
   
   consoleLogger->Log(Color::Green, L"[TTSA] config loaded\n");
   return true;
}

bool TimeThiefServerApp::CreateManagers()
{
   sessionManager_      = std::make_unique<SessionManager>();
   playerManager_       = std::make_unique<PlayerManager>();
   roomDirectory_       = std::make_unique<RoomDirectory>();
   shardManager_        = std::make_unique<ShardManager>();
   matchMaker_          = std::make_unique<MatchMaker>();
   
   if (not gameDataManager_.Init(configReader_->Get())) {
      consoleLogger->Log(Color::Red, L"[TTSA] GameDataManager init fail\n");
      return false;
   }
   
   // TODO: GameShard의 개수 정책 어떻게 할 지 고민해 보아야 함
   const int32 hc = static_cast<int32>(std::thread::hardware_concurrency());
   const int32 shardCount = (hc / 2) > 1 ? (hc / 2) : 1;   // TEMP: Network IO Worker thread의 반절
   
   if (not shardManager_->Init(shardCount, sessionManager_.get(), roomDirectory_.get())) {
      consoleLogger->Log(Color::Red, L"[TTSA] ShardManager init fail\n");
      return false;
   }
   
   if (not matchMaker_->Init(*sessionManager_, *playerManager_, *shardManager_, *roomDirectory_, [this](){ return GenerateRoomId(); })) {
      consoleLogger->Log(Color::Red, L"[TTSA] MatchMaker init fail\n");
      return false;
   }
   
   if (not CreatePacketDispatcher()) {
      consoleLogger->Log(Color::Red, L"[TTSA] CreatePacketDispatcher fail\n");
      return false;
   }
   
   consoleLogger->Log(Color::Green, L"[TTSA] managers created\n");
   return true;
}

bool TimeThiefServerApp::CreateNetworkService()
{
   playerSessionLifecycleService_ = std::make_unique<PlayerSessionLifecycleService>(*sessionManager_, *playerManager_, *shardManager_);
   
   const auto& cfg = configReader_->Get();
   
   std::string serverIPStr = cfg.network.bindIp;
   std::wstring serverIP(serverIPStr.begin(), serverIPStr.end());
   uint16 serverPort = cfg.network.gamePort;
   
   networkService_ = std::make_shared<IocpServerService>(
      NetAddr(serverIP, serverPort),
      [this]() -> std::shared_ptr<SessionBase>
      {
         return std::make_shared<PlayerSession>(*playerSessionLifecycleService_);
      },
      5
   );
   
   if (!networkService_) {
      consoleLogger->Log(Color::Red, L"[TTSA] network service creation fail\n");
      return false;
   }
   
   consoleLogger->Log(Color::Green, L"[TTSA] network service created\n");
   return true;
}

bool TimeThiefServerApp::CreatePacketDispatcher()
{
   packetDispatcher_ = std::make_unique<ServerPacketDispatcher>(
      *sessionManager_,
      *playerManager_,
      *matchMaker_,
      *roomDirectory_,
      shardManager_.get());
   
   SetServerPacketDispatcher(packetDispatcher_.get());
   ServerPacketHandler::Init();
   return true;
}

bool TimeThiefServerApp::StartServices()
{
   if (!networkService_) 
      return false;
   
   if (!networkService_->Start()) {
      consoleLogger->Log(Color::Red, L"[TTSA] network service Start FAIL\n");
      return false;
   }
   
   if (shardManager_) {
      if (not shardManager_->Start(threadManager_)) {
         consoleLogger->Log(Color::Red, L"[TTSA] shard manager Start FAIL\n");
         networkService_->StopService();
         return false;
      }
   }
   
   if (!CreateInitialRooms()) {
      consoleLogger->Log(Color::Red, L"[TTSA] initial room bootstrap FAIL\n");
      networkService_->StopService();
      if (shardManager_) shardManager_->Stop();
      return false;
   }
   
   consoleLogger->Log(Color::Green, L"[TTSA] services started\n");
   return true;
}

void TimeThiefServerApp::LaunchWorkerThreads()
{
   if (!networkService_)
      return;
   
   const int32 hc = static_cast<int32>(std::thread::hardware_concurrency());
   const int32 workerCount = (hc > 1) ? (hc - 1) : 1;
   
   consoleLogger->Log(Color::Green, L"[TTSA] launching worker threads\n");
   
   for (int32 i = 0; i < workerCount; ++i) {
      threadManager_.Launch([this]()
      {
         WorkerLoop();
      });
   }
}

void TimeThiefServerApp::LaunchMatchThread()
{
   threadManager_.Launch([this]()
   {
      MatchLoop();
   });
   
   consoleLogger->Log(Color::Green, L"[TTSA] match thread launched\n");
}

bool TimeThiefServerApp::CreateInitialRooms()
{
   if (!shardManager_ || !roomDirectory_)
      return false;
   
   const RoomId initialRoomId = GenerateRoomId();
   const ShardId initialShardId = shardManager_->SelectShardForNewRoom();
   if (initialRoomId == 0 or initialShardId == 0)
      return false;
   
   bool ok = shardManager_->RequestCreateRoom(initialShardId, CreateRoomParams{ initialRoomId, {} });
   if (!ok) {
      consoleLogger->Log(Color::Red, L"[TTSA] initial room creation fail\n");
      return false;
   }
   
   consoleLogger->Log(Color::Green, L"[TTSA] initial room created\n");
   return true;
}

RoomId TimeThiefServerApp::GenerateRoomId()
{
   return roomIdGenerator_.Generate();
}

void TimeThiefServerApp::WorkerLoop()
{
   while (running_.load()) {
      
      if (networkService_) {
         networkService_->Dispatch(1000);
      }
   }
}

void TimeThiefServerApp::MatchLoop()
{
   while (running_.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      
      if (!running_.load()) 
         break;
      
      if (matchMaker_) {
         matchMaker_->TryMatch();
      }
   }
}
