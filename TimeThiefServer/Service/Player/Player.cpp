#include "pch.h"
#include "Player.h"

bool Player::TrySetNickname(std::string_view nickname)
{
    if (not IsValidNickname(nickname))
        return false;
    
    nickname_.assign(nickname);
    return true;
}

const std::string& Player::GetNickname() const
{
    return nickname_;
}

bool Player::IsValidNickname(std::string_view nickname)
{
    constexpr size_t kMinLen = 2;
    constexpr size_t kMaxLen = 12;
    
    if (nickname.size() < kMinLen || nickname.size() > kMaxLen)
        return false;   // 닉네임 길이 제한
    
    for (char ch : nickname) {
        const bool isAlphaNum = 
            (ch >= 'a' && ch <= 'z') || 
            (ch >= 'A' && ch <= 'Z') || 
            (ch >= '0' && ch <= '9');
        
        const bool isUnderscore = (ch == '_');
        
        if (!isAlphaNum && !isUnderscore)
            return false;   // 허용되지 않는 문자 포함
    }
    
    return true;
}
