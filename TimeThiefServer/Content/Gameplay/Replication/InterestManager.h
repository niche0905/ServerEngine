#pragma once
#include "ReplicationTypes.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

using InterestSet = std::unordered_set<RepObjectId>;

struct InterestResult
{
    std::unordered_map<ClientId, InterestSet> visible;
};

/*-------------------
   InterestManager
-------------------*/
//
// InterestManager는 각 클라이언트가 어떤 오브젝트를 볼 수 있는지 관리한다.
// Replication의 부하를 줄이기 위해 사용된다.
//

class InterestManager
{
public:
    template<typename TRoom>
    InterestResult BuildInterest(const TRoom& room) const
    {
        InterestResult out;
        
        // Room이 아래 두 함수를 제공한다고 가정
        const auto clients = room.GetClients();
        const auto objects = room.GetObjectIds();
        
        for (ClientId cid : clients) {
            auto& set = out.visible[cid];
            for (auto oid : objects) {      // 모든 오브젝트를 일단 추가 (추후 필요한 정보만 담도록)
                set.insert(oid);
            }
        }
        
        return out;
    }
    
};
