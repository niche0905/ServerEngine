#pragma once

namespace se::auth
{
    class C_HandshakeReq;
    class S_HandshakeRes;
}

class PlayerSession;

class IPlayerSessionLifecycle
{
public:
    virtual ~IPlayerSessionLifecycle() = default;
    
public:
    virtual void OnConnected(PlayerSession& session) = 0;
    virtual void OnDisconnected(PlayerSession& session) = 0;
    
    virtual bool HandleHandshake(PlayerSession& session, const se::auth::C_HandshakeReq& pkt) = 0;
    
};
