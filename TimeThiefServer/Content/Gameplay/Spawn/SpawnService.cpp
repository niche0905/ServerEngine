#include "pch.h"
#include "SpawnService.h"

/*-----------------
   SpawnService
-----------------*/

int32 SpawnService::InitialSpawn(ObjectManager& om, uint64 nowMs)
{
}

void SpawnService::Update(ObjectManager& om, uint64 nowMs)
{
}

void SpawnService::NotifyDead(int32 spawnPointId, ObjectId obj, uint64 nowMs)
{
}

int32 SpawnService::DespawnAll(ObjectManager& om, DespawnReason reason, uint64 nowMs)
{
}

bool SpawnService::TrySpawnOne(ObjectManager& om, const SpawnPoint& sp, uint64 nowMs)
{
}

void SpawnService::UpdateRespawn(ObjectManager& om, uint64 nowMs)
{
}

void SpawnService::UpdateDespawn(ObjectManager& om, uint64 nowMs)
{
}
