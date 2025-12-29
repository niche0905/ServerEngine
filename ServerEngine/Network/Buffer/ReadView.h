#pragma once
#include "pch.h"

struct IoVec
{
	const byte*	buffer;
	size_t		length;
};

struct ReadView 
{
    IoVec seg1;   // 첫 연속 구간
    IoVec seg2;   // 환형에서 wrap일 때만 존재, 아니면 len=0
    size_t Total() const noexcept { return seg1.length + seg2.length; }
};
