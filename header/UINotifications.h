#pragma once

#include "settings.h"
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace TradeInGems
{
    namespace UI
    {
        void ShowPerItemNotification(
            const std::vector<std::pair<std::string, int32_t>>& a_removedTotals,
            const std::shared_ptr<const Settings>& a_cfg);

        void ShowTruncatedSummaryNotification(
            const std::vector<std::pair<std::string, int32_t>>& a_removedTotals,
            const std::shared_ptr<const Settings>& a_cfg);

        void ShowMessageBoxNotification(
            const std::vector<std::pair<std::string, int32_t>>& a_removedTotals,
            const std::shared_ptr<const Settings>& a_cfg);

        void ShowConsoleLogNotification(
            const std::vector<std::pair<std::string, int32_t>>& a_removedTotals,
            const std::shared_ptr<const Settings>& a_cfg);

        void DispatchItemNotification(
            ItemNotificationMode a_mode,
            const std::vector<std::pair<std::string, int32_t>>& a_removedTotals,
            const std::shared_ptr<const Settings>& a_cfg);
    }
}