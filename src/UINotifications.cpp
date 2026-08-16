#include "UINotifications.h"
#include <RE/Skyrim.h>
#include <algorithm>
#include <format>

namespace TradeInGems
{
    namespace UI
    {
        void ShowPerItemNotification(const std::vector<std::pair<std::string, int32_t>>& a_removedTotals, const std::shared_ptr<const Settings>& a_cfg)
        {
            for (auto const& [name, total] : a_removedTotals) {
                RE::DebugNotification(TradeInGems::SafeFormat(a_cfg->msgTradedPerItem, TradeInGems::Defaults::msgTradedPerItem,
                    a_cfg->colorSystemHeader, total, a_cfg->colorLootAsset, name).c_str());
            }
        }

        void ShowTruncatedSummaryNotification(const std::vector<std::pair<std::string, int32_t>>& a_removedTotals, const std::shared_ptr<const Settings>& a_cfg)
        {
            std::string batchedItems = "";
            size_t itemCount = 0;
            size_t hiddenCount = 0;
            const size_t maxNameLength = 16;

            for (auto const& [name, total] : a_removedTotals) {
                if (itemCount < 3) {
                    if (itemCount > 0) batchedItems += ", ";
                    std::string cleanName = name;
                    if (cleanName.length() > maxNameLength) {
                        cleanName = cleanName.substr(0, maxNameLength - 2) + ".";
                    }
                    batchedItems += std::format("<font color='#{}'>{}x {}</font>", a_cfg->colorLootAsset, total, cleanName);
                }
                else {
                    hiddenCount++;
                }
                itemCount++;
            }

            if (hiddenCount > 0) {
                batchedItems += TradeInGems::SafeFormat(a_cfg->msgTruncatedMore, TradeInGems::Defaults::msgTruncatedMore, a_cfg->colorSystemHeader, hiddenCount);
            }
            if (!batchedItems.empty()) {
                std::string finalMsg = TradeInGems::SafeFormat(a_cfg->msgTruncatedHeader, TradeInGems::Defaults::msgTruncatedHeader, a_cfg->colorSystemHeader, batchedItems);
                RE::DebugNotification(finalMsg.c_str());
            }
        }

        void ShowMessageBoxNotification(const std::vector<std::pair<std::string, int32_t>>& a_removedTotals, const std::shared_ptr<const Settings>& a_cfg)
        {
            std::string headerPart = TradeInGems::SafeFormat(a_cfg->msgMsgBoxHeader, TradeInGems::Defaults::msgMsgBoxHeader, a_cfg->currencyName);
            std::string msgBody = std::format("{}\n{}", headerPart, a_cfg->msgMsgBoxLimiter);

            size_t totalWidth = a_cfg->msgMsgBoxLimiter.length();
            if (totalWidth == 0) totalWidth = 30;
            size_t entryWidth = totalWidth / 2 + 6;

            std::vector<std::string> rawLines;

            for (auto const& [name, total] : a_removedTotals) {
                std::string cleanName = name;
                size_t digitCount = std::to_string(total).length();

                size_t prefixLen = digitCount + 5;
                size_t allowedNameLength = (entryWidth > prefixLen) ? (entryWidth - prefixLen) : 2;

                if (cleanName.length() > allowedNameLength) {
                    allowedNameLength = (allowedNameLength >= 2) ? allowedNameLength : 2;
                    cleanName = cleanName.substr(0, allowedNameLength - 2) + ".";
                }

                std::string itemStr = std::format(" {}x {}", total, cleanName);

                if (itemStr.length() < entryWidth) {
                    itemStr.append(entryWidth - itemStr.length(), ' ');
                }
                else if (itemStr.length() > entryWidth) {
                    itemStr = itemStr.substr(0, entryWidth);
                }

                rawLines.push_back(itemStr);
            }

            const size_t maxRows = 7;
            const size_t maxTotalItems = maxRows * 2;
            size_t rowsToRender = std::min(maxRows, (rawLines.size() + 1) / 2);
            for (size_t i = 0; i < rowsToRender; ++i) {
                size_t idx1 = i * 2;
                size_t idx2 = i * 2 + 1;

                std::string leftCol = (idx1 < rawLines.size()) ? rawLines[idx1] : "";
                std::string rightCol = (idx2 < rawLines.size()) ? rawLines[idx2] : "";

                if (!rightCol.empty()) {
                    msgBody += std::format("\n{}{}", leftCol, rightCol);
                }
                else if (!leftCol.empty()) {
                    msgBody += std::format("\n{}", leftCol);
                }
            }

            if (rawLines.size() > maxTotalItems) {
                size_t truncatedCount = rawLines.size() - maxTotalItems;
                std::string truncatedText = TradeInGems::SafeFormat(a_cfg->msgMsgBoxTruncated, TradeInGems::Defaults::msgMsgBoxTruncated, truncatedCount, a_cfg->currencyName);
                msgBody += std::format("\n\n{}", truncatedText);
            }

            RE::DebugMessageBox(msgBody.c_str());
        }

        void ShowConsoleLogNotification(const std::vector<std::pair<std::string, int32_t>>& a_removedTotals, const std::shared_ptr<const Settings>& a_cfg)
        {
            if (auto* console = RE::ConsoleLog::GetSingleton()) {
                console->Print("[TIG-LACE] List of traded %s:", a_cfg->currencyName.c_str());
                std::string rowBuffer = "";
                size_t itemsInCurrentRow = 0;
                const size_t maxNameLength = 18;

                for (auto const& [name, total] : a_removedTotals) {
                    std::string cleanName = name;
                    if (cleanName.length() > maxNameLength) {
                        cleanName = cleanName.substr(0, maxNameLength - 2) + "..";
                    }
                    std::string itemFragment = std::format(" {}x {}", total, cleanName);
                    if (itemsInCurrentRow == 0) {
                        rowBuffer = " - " + itemFragment;
                    }
                    else {
                        rowBuffer += "  |  " + itemFragment;
                    }
                    itemsInCurrentRow++;
                    if (itemsInCurrentRow == 6) {
                        console->Print("%s", rowBuffer.c_str());
                        rowBuffer = "";
                        itemsInCurrentRow = 0;
                    }
                }
                if (itemsInCurrentRow > 0 && !rowBuffer.empty()) {
                    console->Print("%s", rowBuffer.c_str());
                }
            }
        }

        void DispatchItemNotification(ItemNotificationMode a_mode, const std::vector<std::pair<std::string, int32_t>>& a_removedTotals, const std::shared_ptr<const Settings>& a_cfg)
        {
            switch (a_mode) {
            case ItemNotificationMode::kPerItem:
                ShowPerItemNotification(a_removedTotals, a_cfg);
                break;
            case ItemNotificationMode::kTruncatedSummary:
                ShowTruncatedSummaryNotification(a_removedTotals, a_cfg);
                break;
            case ItemNotificationMode::kMessageBoxPopup:
                ShowMessageBoxNotification(a_removedTotals, a_cfg);
                break;
            case ItemNotificationMode::kConsoleLogOnly:
                ShowConsoleLogNotification(a_removedTotals, a_cfg);
                break;
            }
        }
    }
}