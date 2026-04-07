#pragma once
#include "PlayerSessionState.h"
#include "Network/Session/IOCP/IocpSession.h"
#include "Protocol/Framing/PacketHeader.h"
#include "Protocol.pb.h"

class IPlayerSessionLifecycle;

/*-----------------
   PlayerSession
-----------------*/
//
// PlayerSession은 게임 플레이어와의 네트워크 세션을 관리합니다.
//

// TODO: Temp로 IocpSession 상속, 추후 필요에 따라 기능 추가 예정
class PlayerSession : public IocpSession
{
public:
   PlayerSession(IPlayerSessionLifecycle& lifecycle);
   virtual ~PlayerSession() override;
   
   void Dispatch(class IIoEvent* ioEvent, int32 numOfBytes) override;
   
   // TEMP: 간단한 문자열 통신 테스트
   int32 GetPacketHeaderSize() const override
   {
      return sizeof(Protocol::Framing::PacketHeader);
   }
   int32 GetPacketSize(byte* packet) const override
   {
      return reinterpret_cast<uint16*>(packet)[0];
   }
   
   bool CanPacketProcess(const byte* buffer, int32 len) override;
   void OnRecvPacket(byte* buffer, int32 len) override;
   
public:
   PlayerSessionState GetState() const { return state_; }
   void SetState(PlayerSessionState newState) { state_ = newState; }
   
   PlayerId GetPlayerId() const { return playerId_; }
   void SetPlayerId(PlayerId playerId) { playerId_ = playerId; }
   
public:
   bool HandleHandshake(const se::auth::C_HandshakeReq& pkt);
   
// on event interface for content override
protected:
   void OnConnected() override;
   void OnDisconnected() override;
   // int32 OnRecv(byte* buffer, int32 len) override; // OnRecv is already writed in PacketSession, to write only OnRecvPacket
   void OnSend(int32 len) override;
   
private:
   IPlayerSessionLifecycle& lifecycle_;
   
   PlayerSessionState state_ = PlayerSessionState::Connected;
   
   PlayerId playerId_ = 0;   // Caching
   
   // TODO: 플레이어 세션 관련 멤버 변수 추가 예정
    
};
