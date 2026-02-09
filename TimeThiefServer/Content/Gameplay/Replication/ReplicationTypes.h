#pragma once
#include "Content/Object/ObjectId.h"

struct SE::Math::Vector3;

using RepObjectId = ObjectId;
using ClientId = uint32;
using TickSeq = uint32;

struct RepFrame
{
    TickSeq seq{0};
    uint64 nowMs{0};
};

enum class RepChannel : uint8
{
    Spawn = 0,
    Despawn,
    State,
    Event,
};
