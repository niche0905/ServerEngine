#pragma once

struct CreateRoomParams
{
    RoomId roomId = 0;
    std::vector<PlayerId> playerIds;
};
