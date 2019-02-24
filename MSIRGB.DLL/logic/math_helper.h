#pragma once

#include "pch.h"

inline std::uint8_t fast_ceil(std::uint8_t num, std::uint8_t den) 
{
    return (num / den) + !!(num % den);
}