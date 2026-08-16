#pragma once

#include "SKSEMenuFramework.h"
#include "settings.h"
#include <atomic>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace TradeInGems
{
    inline constexpr auto kIniPath = L"Data/SKSE/Plugins/TradeInGemsLACE/TradeInGemsLACE.ini";
    inline constexpr auto kBackupIniPath = L"Data/SKSE/Plugins/TradeInGemsLACE/SKSEMenu/TradeInGemsLACE_Backup.ini";

    struct MenuTextsMCP {
        std::string menuConfiguration{ "Configuration" };
        std::string tabEconomy{ "Economy and Gameplay Settings" };
        std::string tabColors{ "Colors and Display" };
        std::string tabLabels{ "Label Naming" };
        std::string liveEnabled{ "Trading Mode Active in Live Session" };
        std::string bootEnabled{ "Trading Mode Active in Boot and New Game" };
        std::string toggleKey{ "Trading Toggle Key" };
        std::string wealthKey{ "Wealth Check Key" };
        std::string itemValueMode{ "Item Value Mode" };
        std::string spendingOrderMode{ "Spending Priority" };
        std::string itemValueMode0{ "Currency (Buy Price)" };
        std::string itemValueMode1{ "Barter (Sell Price)" };
        std::string itemValueMode2{ "Fixed (Base Value)" };
        std::string spendingPriority0{ "Lowest Value/Weight" };
        std::string spendingPriority1{ "Highest Value/Weight" };
        std::string spendingPriority2{ "Heaviest" };
        std::string spendingPriority3{ "Lightest" };
        std::string spendingPriority4{ "Cheapest" };
        std::string spendingPriority5{ "Most Expensive" };
        std::string spendingPriority6{ "None" };
        std::string barterValueMultiplier{ "Barter Mode Multiplier" };
        std::string spendItemsBeforeGold{ "Spend Loot/Items Before Gold" };
        std::string cursedGoldMode{ "Cursed Gold Challenge Mode" };
        std::string includePowerModifiers{ "Include Speechcraft Power Modifiers" };
        std::string includePerkModifiers{ "Include Speechcraft Perk Modifiers" };
        std::string blockFollowerInjection{ "Block Follower Gold Injection" };
        std::string useActivateHandler{ "Early Gold Injection on Activate" };
        std::string colorsHeader{ "Pick your notification colors." };
        std::string colorStatusActive{ "Active Status" };
        std::string colorStatusInactive{ "Inactive Status" };
        std::string colorGoldAsset{ "Gold Asset" };
        std::string colorLootAsset{ "Item Asset" };
        std::string colorTotalAsset{ "Total Asset" };
        std::string colorExpectedValue{ "Expected Value" };
        std::string colorSystemHeader{ "System Header" };
        std::string itemNotificationMode{ "Traded item Notification Mode" };
        std::string itemNotificationMode0{ "Default" };
        std::string itemNotificationMode1{ "Truncated" };
        std::string itemNotificationMode2{ "Message Box" };
        std::string itemNotificationMode3{ "Console" };
        std::string showModeStatus{ "Display Modes info line" };
        std::string showExpectedValue{ "Display Expected Value line" };
        std::string enableDebug{ "Enable Debug Notifications" };
        std::string labelsHeader{ "Edit labels used by the mod." };
        std::string currencyName{ "Currency Name" };
        std::string labelGold{ "Gold Label" };
        std::string labelGoldAbbr{ "Gold Abbreviation" };
        std::string labelOn{ "ON Label" };
        std::string labelOff{ "OFF Label" };
        std::string labelActive{ "ACTIVE Label" };
        std::string labelInactive{ "INACTIVE Label" };
        std::string labelModeFixed{ "Fixed Mode Label" };
        std::string labelModeCurrency{ "Currency Mode Label" };
        std::string labelModeBarter{ "Barter Mode Label" };
        std::string unsavedPending{ "* Unsaved Changes Pending" };
        std::string btnApply{ "Apply changes and reload INI" };
        std::string btnDiscard{ "Discard Unsaved Changes" };
        std::string btnRestore{ "Restore Backup Settings" };
        std::string confirmRestore{ "Restore active tab defaults?" };
        std::string btnYesRestore{ "Yes, Restore" };
        std::string btnCancel{ "Cancel" };
        std::string savedSuccess{ "Settings applied and saved successfully!" };
    };

    extern std::atomic<std::shared_ptr<const MenuTextsMCP>> g_menuTexts;

    void LoadMenuTexts();
    void LogMenuFrameworkStatus(const std::string& a_message);

    struct KeyOption {
        uint32_t code;
        std::string label;
        std::string category;
    };

    const std::vector<KeyOption>& GetKeyOptionsList();
    std::string GetKeyName(uint32_t a_key);
    bool CaseInsensitiveContains(std::string_view haystack, std::string_view needle);
    bool ImGuiSearchableKeyPicker(const char* a_label, uint32_t& a_selectedKeyCode, std::string& a_filterState);
    void EnsureBackupIniExists();
    void SaveSettings(bool a_bEnabledBoot, const Settings& cfg);
    bool ImGuiInputString(const char* label, std::string& strValue);
    void HexToRGB(const std::string& a_hex, float a_rgb[3]);
    std::string RGBToHex(const float a_rgb[3]);
    bool ImGuiColorHexEdit(const char* label, std::string& hexValue);

    class MCPLaceMenu
    {
    public:
        static bool Register();
        static bool s_isDirty;

    private:
        static void __stdcall OnMenuOpen(SKSEMenuFramework::Model::EventType a_event);
        static const char* GetItemValueModeText(const MenuTextsMCP& txt, int mode);
        static const char* GetSpendingPriorityText(const MenuTextsMCP& txt, int mode);
        static const char* GetItemNotificationModeText(const MenuTextsMCP& a_txt, int32_t a_mode);
        static void ResetUIBufferFromMemory();
        static bool IsDirty();
        static void RestoreTabBackup(int activeTab);
        static void RenderMenu();

        static Settings s_menuSettings;
        static bool s_bEnabledBoot;
        static bool s_savedBootEnabled;
        static bool s_liveEnabled;
        static bool s_bootStateInitialized;
        static float s_savedTimer;

        static std::string s_toggleKeySearch;
        static std::string s_wealthKeySearch;

        static int s_activeTab;
        static bool s_confirmRestoreTab;
    };
}