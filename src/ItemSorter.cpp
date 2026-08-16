#include "ItemSorter.h"
#include <RE/Skyrim.h>
#include <algorithm>

namespace TradeInGems
{
    void SortItemsForSpending(std::vector<SortableItem>& a_items, int32_t a_mode)
    {
        auto mode = static_cast<SpendingOrderMode>(a_mode);

        if (mode == SpendingOrderMode::kNoSorting) {
            return;
        }

        std::sort(a_items.begin(), a_items.end(), [mode](const SortableItem& a, const SortableItem& b) {
            float weightA = a.weight > 0.0f ? a.weight : 0.001f;
            float weightB = b.weight > 0.0f ? b.weight : 0.001f;

            float ratioA = a.baseValue / weightA;
            float ratioB = b.baseValue / weightB;

            switch (mode) {
            case SpendingOrderMode::kWorstWeightToValue:
                return ratioA < ratioB;
            case SpendingOrderMode::kBestWeightToValue:
                return ratioA > ratioB;
            case SpendingOrderMode::kHeaviestFirst:
                return a.weight > b.weight;
            case SpendingOrderMode::kLightestFirst:
                return a.weight < b.weight;
            case SpendingOrderMode::kCheapestFirst:
                return a.baseValue < b.baseValue;
            case SpendingOrderMode::kMostExpensiveFirst:
                return a.baseValue > b.baseValue;
            default:
                return false;
            }
            });
    }
}