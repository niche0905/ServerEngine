#pragma once
#include "SavePointSnapshot.h"
#include "Content/Shared/BaseComponent.h"

/*-----------------
   SaveComponent
-----------------*/
//
// SaveComponent는 Player의 세이브 포인트 기능에 대해서 저장해야 할 정보들을 관리하는 컴포넌트입니다.
//

class SaveComponent : public BaseComponent
{
public:
   void Init(BaseObject* owner);
   
   bool CaptureSnapshot();
   bool Rollback();
   
private:
   SavePointSnapshot snapshot_{};
    
};
