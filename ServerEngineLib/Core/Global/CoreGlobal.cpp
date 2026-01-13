#include "pch.h"
#include "CoreGlobal.h"
#include "Utils/Log/ConsoleLogger.h"

/*--------------
   CoreGlobal
--------------*/
//
// CoreGlobal는 글로벌 설정 및 상태를 관리하는 클래스입니다
// 이 클래스는 애플리케이션 전반에 걸쳐 사용되는 전역 변수와 설정을 포함합니다
//

ConsoleLogger* consoleLogger = nullptr;

class CoreGlobal
{
public:
    CoreGlobal()
    {
        std::wcout << L"CoreGlobal Initial" << std::endl;
        std::wcout << L"Test Dependency" << std::endl;
        std::wcout << L"Test Dependency2222" << std::endl;
        
        consoleLogger = new ConsoleLogger();      // 콘솔 로거 초기화(생성)
      
        SocketUtils::Initialize();                // 소켓 유틸리티 초기화
    }
   
    ~CoreGlobal()
    {
        std::wcout << L"CoreGlobal Destruct" << std::endl;

        delete consoleLogger;                     // 콘솔 로거 해제(삭제)
        consoleLogger = nullptr;
        
        SocketUtils::Clean();                   // 소켓 유틸리티 정리(종료)
    }
    
} coreGlobalInstance;  // 전역 인스턴스 생성
