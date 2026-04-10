#pragma once

enum class RoomState : uint8
{
    WaitingForPlayers,
    Loading,
    Playing,
    Ending,
    Closed
};
