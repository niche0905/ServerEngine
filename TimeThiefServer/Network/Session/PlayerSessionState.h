#pragma once

enum class PlayerSessionState : uint8
{
    None = 0,
    
    Connected,          // TCP 연결 직후
    Handshaking,        // Handshake 요청/응답 대기
    InLobby,            // Handshake 성공, Lobby 상태
    InRoom,             // 방 입장 상태
    
    Closing,            // 종료 처리 중
    Closed,             // 완전 종료
    
};
