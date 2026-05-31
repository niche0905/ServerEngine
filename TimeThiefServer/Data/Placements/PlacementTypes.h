#pragma once

struct PlacementTransform
{
   SE::Math::Vector3 position;
   float yaw = 0.0f;
};

struct StorePlacement
{
   PlacementTransform transform;
};

struct ChestPlacement
{
   PlacementTransform transform;
   int32 lootTableId = 1;
};

struct InteractionPlacementData
{
   std::vector<StorePlacement> stores;
   std::vector<ChestPlacement> chests;
};

struct MonsterSpawnGroupPlacement
{
   uint32 templateId = 0;
   uint32 spawnCount = 0;
   bool isBoss = false;
   std::vector<PlacementTransform> spawnCandidates;
};

struct MonsterPlacementData
{
   std::vector<MonsterSpawnGroupPlacement> spawnGroups;
};

struct MapPlacementData
{
   InteractionPlacementData interactions;
   MonsterPlacementData monsters;
};
