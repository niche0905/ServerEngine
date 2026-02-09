#pragma once

enum class RepField : uint8
{
    Transform = 0,
    Health,
    Effects,
    Inventory,
    Wallet,
    Interaction,
    
    // 확장 하도록 64개 까지만 허용
    
    _Count
};

using RepMask = uint64;

constexpr RepMask RepBit(RepField f)
{
    return (RepMask{1} << static_cast<uint8>(f));
}

constexpr bool RepMaskHas(RepMask mask, RepField f)
{
    return (mask & RepBit(f)) != 0;
}

constexpr void RepMaskSet(RepMask& mask, RepField f)
{
    mask |= RepBit(f);
}

constexpr void RepMaskClear(RepMask& mask, RepField f)
{
    mask &= ~RepBit(f);
}
