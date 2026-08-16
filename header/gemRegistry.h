#pragma once
#include <RE/Skyrim.h>
#include <unordered_map>

namespace TradeInGems {
    void AppendModdedGems(std::unordered_map<RE::FormID, int32_t>& a_gemMap);
}