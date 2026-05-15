#pragma once

enum class PlayerSessionState : uint8
{
    None = 0,
    
    Created,            // 세션 객체 생성 직후, TCP 연결 전
    Connected,          // TCP 연결 직후
    Handshaking,        // Handshake 요청/응답 대기
    InLobby,            // Handshake 성공, Lobby 상태
    
    MatchMaking,        // 매칭 대기 상태
    MatchingSucc,       // 매칭 성공, Room 입장 대기 상태
    
    InRoom,             // 방 입장 상태
    
    Closing,            // 종료 처리 중
    Closed,             // 완전 종료
    
};
