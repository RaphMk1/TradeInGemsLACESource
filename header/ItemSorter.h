#pragma once

#include "Settings.h"
#include <cstdint>
#include <vector>

namespace RE
{
    class TESBoundObject;
}

namespace TradeInGems
{
    struct SortableItem
    {
        RE::TESBoundObject* item{ nullptr };
        int32_t count{ 0 };
        float baseValue{ 0.0f };
        float weight{ 0.0f };
    };

    void SortItemsForSpending(std::vector<SortableItem>& a_items, int32_t a_mode);
}