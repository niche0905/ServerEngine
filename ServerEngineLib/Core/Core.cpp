#include "pch.h"
#include "Core.h"
#include <mutex>
#include "Physics/Narrowphase/IntersectTable.h"

namespace SE
{
    void Init()
    {
        static std::once_flag InitFlag;
        std::call_once(InitFlag, []
        {
            SE::Physics::Narrowphase::InitIntersectTable();
            SocketUtils::Initialize();                // 소켓 유틸리티 초기화
            
            std::atexit([]
            {
                SocketUtils::Clean();                   // 소켓 유틸리티 정리(종료)
            });
        });
    }
}
