#pragma once


using PlayerId = uint64;
using SessionId = uint64;
using PawnId = uint32;
using ShardId = uint32;
using RoomId = uint32;

using ItemId = uint32;
using SkillId = uint32;

constexpr size_t MaxWeaponSlots = 3;
constexpr int32 MaxActiveSkills = 2;   // 플레이어가 동시에 장착할 수 있는 스킬 개수

using WeaponUpgradeCode = uint32;
using StatUpgradeCode = uint32;

using TimerId = uint64;

using Job = std::function<void()>;
