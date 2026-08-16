#include "MCPLaceMenu.h"
#include <SimpleIni.h>
#include <spdlog/spdlog.h>
#include <RE/Skyrim.h>
#include <format>
#include <cmath>
#include <charconv>
#include <algorithm>
#include <cctype>

namespace TradeInGems
{
    std::atomic<std::shared_ptr<const MenuTextsMCP>> g_menuTexts{ std::make_shared<MenuTextsMCP>() };

    bool MCPLaceMenu::s_isDirty{ false };
    Settings MCPLaceMenu::s_menuSettings{};
    bool MCPLaceMenu::s_bEnabledBoot{ true };
    bool MCPLaceMenu::s_savedBootEnabled{ true };
    bool MCPLaceMenu::s_liveEnabled{ true };
    bool MCPLaceMenu::s_bootStateInitialized{ false };
    float MCPLaceMenu::s_savedTimer{ 0.0f };
    std::string MCPLaceMenu::s_toggleKeySearch{};
    std::string MCPLaceMenu::s_wealthKeySearch{};
    int MCPLaceMenu::s_activeTab{ 0 };
    bool MCPLaceMenu::s_confirmRestoreTab{ false };

    void LoadMenuTexts() {
        CSimpleIniA ini;
        ini.SetUnicode();

        const auto path = L"Data/SKSE/Plugins/TradeInGemsLACE/SKSEMenu/MenuTextsMCP.ini";

        if (ini.LoadFile(path) >= 0) {
            auto newTexts = std::make_shared<MenuTextsMCP>();

            newTexts->menuConfiguration = ini.GetValue("MenuTexts", "sMenuConfiguration", "Configuration");
            newTexts->tabEconomy = ini.GetValue("MenuTexts", "sTabEconomy", "Economy and Gameplay Settings");
            newTexts->tabColors = ini.GetValue("MenuTexts", "sTabColors", "Colors and Display");
            newTexts->tabLabels = ini.GetValue("MenuTexts", "sTabLabels", "Label Naming");
            newTexts->liveEnabled = ini.GetValue("MenuTexts", "sLiveEnabled", "Trading Mode Active in Live Session");
            newTexts->bootEnabled = ini.GetValue("MenuTexts", "sBootEnabled", "Trading Mode Active in Boot and New Game");
            newTexts->toggleKey = ini.GetValue("MenuTexts", "sToggleKey", "Trading Toggle Key");
            newTexts->wealthKey = ini.GetValue("MenuTexts", "sWealthKey", "Wealth Check Key");
            newTexts->itemValueMode = ini.GetValue("MenuTexts", "sItemValueMode", "Item Value Mode");
            newTexts->spendingOrderMode = ini.GetValue("MenuTexts", "sSpendingOrderMode", "Spending Priority");
            newTexts->itemValueMode0 = ini.GetValue("MenuTexts", "sItemValueMode0", "Currency (Buy Price)");
            newTexts->itemValueMode1 = ini.GetValue("MenuTexts", "sItemValueMode1", "Barter (Sell Price)");
            newTexts->itemValueMode2 = ini.GetValue("MenuTexts", "sItemValueMode2", "Fixed (Base Value)");
            newTexts->spendingPriority0 = ini.GetValue("MenuTexts", "sSpendingPriority0", "Lowest Value/Weight");
            newTexts->spendingPriority1 = ini.GetValue("MenuTexts", "sSpendingPriority1", "Highest Value/Weight");
            newTexts->spendingPriority2 = ini.GetValue("MenuTexts", "sSpendingPriority2", "Heaviest");
            newTexts->spendingPriority3 = ini.GetValue("MenuTexts", "sSpendingPriority3", "Lightest");
            newTexts->spendingPriority4 = ini.GetValue("MenuTexts", "sSpendingPriority4", "Cheapest");
            newTexts->spendingPriority5 = ini.GetValue("MenuTexts", "sSpendingPriority5", "Most Expensive");
            newTexts->spendingPriority6 = ini.GetValue("MenuTexts", "sSpendingPriority6", "None");
            newTexts->barterValueMultiplier = ini.GetValue("MenuTexts", "sBarterValueMultiplier", "Barter Mode Multiplier");
            newTexts->spendItemsBeforeGold = ini.GetValue("MenuTexts", "sSpendItemsBeforeGold", "Spend Items Before Gold");
            newTexts->cursedGoldMode = ini.GetValue("MenuTexts", "sCursedGoldMode", "Cursed Gold Mode");
            newTexts->includePowerModifiers = ini.GetValue("MenuTexts", "sIncludePowerModifiers", "Include Price Power Modifiers");
            newTexts->includePerkModifiers = ini.GetValue("MenuTexts", "sIncludePerkModifiers", "Include Price Perk Modifiers");
            newTexts->blockFollowerInjection = ini.GetValue("MenuTexts", "sBlockFollowerInjection", "Block Follower Gold Injection");
            newTexts->useActivateHandler = ini.GetValue("MenuTexts", "sUseActivateHandler", "Early Gold Injection on Activate");
            newTexts->colorsHeader = ini.GetValue("MenuTexts", "sColorsHeader", "Pick your notification colors.");
            newTexts->colorStatusActive = ini.GetValue("MenuTexts", "sColorStatusActive", "Active Status");
            newTexts->colorStatusInactive = ini.GetValue("MenuTexts", "sColorStatusInactive", "Inactive Status");
            newTexts->colorGoldAsset = ini.GetValue("MenuTexts", "sColorGoldAsset", "Gold Asset");
            newTexts->colorLootAsset = ini.GetValue("MenuTexts", "sColorLootAsset", "Item Asset");
            newTexts->colorTotalAsset = ini.GetValue("MenuTexts", "sColorTotalAsset", "Total Asset");
            newTexts->colorExpectedValue = ini.GetValue("MenuTexts", "sColorExpectedValue", "Expected Value");
            newTexts->colorSystemHeader = ini.GetValue("MenuTexts", "sColorSystemHeader", "System Header");
            newTexts->itemNotificationMode = ini.GetValue("MenuTexts", "sItemNotificationMode", "Traded item Notification Mode");
            newTexts->itemNotificationMode0 = ini.GetValue("MenuTexts", "sItemNotificationMode0", "Default");
            newTexts->itemNotificationMode1 = ini.GetValue("MenuTexts", "sItemNotificationMode1", "Truncated");
            newTexts->itemNotificationMode2 = ini.GetValue("MenuTexts", "sItemNotificationMode2", "Message Box");
            newTexts->itemNotificationMode3 = ini.GetValue("MenuTexts", "sItemNotificationMode3", "Console");
            newTexts->showModeStatus = ini.GetValue("MenuTexts", "sShowModeStatus", "Display Modes info line");
            newTexts->showExpectedValue = ini.GetValue("MenuTexts", "sShowExpectedValue", "Display Expected Value line");
            newTexts->enableDebug = ini.GetValue("MenuTexts", "sEnableDebug", "Enable Debug Notifications");
            newTexts->labelsHeader = ini.GetValue("MenuTexts", "sLabelsHeader", "Edit labels used by the mod.");
            newTexts->currencyName = ini.GetValue("MenuTexts", "sCurrencyName", "Currency Name");
            newTexts->labelGold = ini.GetValue("MenuTexts", "sLabelGold", "Gold Label");
            newTexts->labelGoldAbbr = ini.GetValue("MenuTexts", "sLabelGoldAbbr", "Gold Abbreviation");
            newTexts->labelOn = ini.GetValue("MenuTexts", "sLabelOn", "ON Label");
            newTexts->labelOff = ini.GetValue("MenuTexts", "sLabelOff", "OFF Label");
            newTexts->labelActive = ini.GetValue("MenuTexts", "sLabelActive", "ACTIVE Label");
            newTexts->labelInactive = ini.GetValue("MenuTexts", "sLabelInactive", "INACTIVE Label");
            newTexts->labelModeFixed = ini.GetValue("MenuTexts", "sLabelModeFixed", "Fixed Mode Label");
            newTexts->labelModeCurrency = ini.GetValue("MenuTexts", "sLabelModeCurrency", "Currency Mode Label");
            newTexts->labelModeBarter = ini.GetValue("MenuTexts", "sLabelModeBarter", "Barter Mode Label");
            newTexts->unsavedPending = ini.GetValue("MenuTexts", "sUnsavedPending", "* Unsaved Changes Pending");
            newTexts->btnApply = ini.GetValue("MenuTexts", "sBtnApply", "Save and Apply Settings");
            newTexts->btnDiscard = ini.GetValue("MenuTexts", "sBtnDiscard", "Discard Unsaved Changes");
            newTexts->btnRestore = ini.GetValue("MenuTexts", "sBtnRestore", "Restore Backup Settings");
            newTexts->confirmRestore = ini.GetValue("MenuTexts", "sConfirmRestore", "Restore active tab entries?");
            newTexts->btnYesRestore = ini.GetValue("MenuTexts", "sBtnYesRestore", "Yes, Restore");
            newTexts->btnCancel = ini.GetValue("MenuTexts", "sBtnCancel", "Cancel");
            newTexts->savedSuccess = ini.GetValue("MenuTexts", "sSavedSuccess", "Settings applied and saved!");

            g_menuTexts.store(newTexts);
        }
    }

    void LogMenuFrameworkStatus(const std::string& a_message) {
        if (auto logger = spdlog::get("ConsoleScan")) {
            logger->info("[MenuFramework] {}", a_message);
        }
        else {
            SKSE::log::info("[MenuFramework] {}", a_message);
        }
    }

    const std::vector<KeyOption>& GetKeyOptionsList() {
        static const std::vector<KeyOption> keys = {
            { 0x01, "Escape", "Keyboard" }, { 0x02, "1", "Keyboard" }, { 0x03, "2", "Keyboard" }, { 0x04, "3", "Keyboard" },
            { 0x05, "4", "Keyboard" }, { 0x06, "5", "Keyboard" }, { 0x07, "6", "Keyboard" }, { 0x08, "7", "Keyboard" },
            { 0x09, "8", "Keyboard" }, { 0x0A, "9", "Keyboard" }, { 0x0B, "0", "Keyboard" }, { 0x0C, "Minus (-)", "Keyboard" },
            { 0x0D, "Equals (=)", "Keyboard" }, { 0x0E, "Backspace", "Keyboard" }, { 0x0F, "Tab", "Keyboard" },
            { 0x10, "Q", "Keyboard" }, { 0x11, "W", "Keyboard" }, { 0x12, "E", "Keyboard" }, { 0x13, "R", "Keyboard" },
            { 0x14, "T", "Keyboard" }, { 0x15, "Y", "Keyboard" }, { 0x16, "U", "Keyboard" }, { 0x17, "I", "Keyboard" },
            { 0x18, "O", "Keyboard" }, { 0x19, "P", "Keyboard" }, { 0x1A, "Left Bracket ([)", "Keyboard" },
            { 0x1B, "Right Bracket (])", "Keyboard" }, { 0x1C, "Enter", "Keyboard" }, { 0x1D, "Left Ctrl", "Keyboard" },
            { 0x1E, "A", "Keyboard" }, { 0x1F, "S", "Keyboard" }, { 0x20, "D", "Keyboard" }, { 0x21, "F", "Keyboard" },
            { 0x22, "G", "Keyboard" }, { 0x23, "H", "Keyboard" }, { 0x24, "J", "Keyboard" }, { 0x25, "K", "Keyboard" },
            { 0x26, "L", "Keyboard" }, { 0x27, "Semicolon (;)", "Keyboard" }, { 0x28, "Apostrophe (')", "Keyboard" },
            { 0x29, "Grave (`)", "Keyboard" }, { 0x2A, "Left Shift", "Keyboard" }, { 0x2B, "Backslash (\\)", "Keyboard" },
            { 0x2C, "Z", "Keyboard" }, { 0x2D, "X", "Keyboard" }, { 0x2E, "C", "Keyboard" }, { 0x2F, "V", "Keyboard" },
            { 0x30, "B", "Keyboard" }, { 0x31, "N", "Keyboard" }, { 0x32, "M", "Keyboard" }, { 0x33, "Comma (,)", "Keyboard" },
            { 0x34, "Period (.)", "Keyboard" }, { 0x35, "Slash (/)", "Keyboard" }, { 0x36, "Right Shift", "Keyboard" },
            { 0x37, "Numpad *", "Keyboard" }, { 0x38, "Left Alt", "Keyboard" }, { 0x39, "Space", "Keyboard" },
            { 0x3A, "Caps Lock", "Keyboard" }, { 0x3B, "F1", "Keyboard" }, { 0x3C, "F2", "Keyboard" }, { 0x3D, "F3", "Keyboard" },
            { 0x3E, "F4", "Keyboard" }, { 0x3F, "F5", "Keyboard" }, { 0x40, "F6", "Keyboard" }, { 0x41, "F7", "Keyboard" },
            { 0x42, "F8", "Keyboard" }, { 0x43, "F9", "Keyboard" }, { 0x44, "F10", "Keyboard" }, { 0x45, "Num Lock", "Keyboard" },
            { 0x46, "Scroll Lock", "Keyboard" }, { 0x47, "Numpad 7", "Keyboard" }, { 0x48, "Numpad 8", "Keyboard" },
            { 0x49, "Numpad 9", "Keyboard" }, { 0x4A, "Numpad -", "Keyboard" }, { 0x4B, "Numpad 4", "Keyboard" },
            { 0x4C, "Numpad 5", "Keyboard" }, { 0x4D, "Numpad 6", "Keyboard" }, { 0x4E, "Numpad +", "Keyboard" },
            { 0x4F, "Numpad 1", "Keyboard" }, { 0x50, "Numpad 2", "Keyboard" }, { 0x51, "Numpad 3", "Keyboard" },
            { 0x52, "Numpad 0", "Keyboard" }, { 0x53, "Numpad .", "Keyboard" }, { 0x57, "F11", "Keyboard" },
            { 0x58, "F12", "Keyboard" }, { 0x9C, "Numpad Enter", "Keyboard" }, { 0x9D, "Right Ctrl", "Keyboard" },
            { 0xB5, "Numpad /", "Keyboard" }, { 0xB8, "Right Alt", "Keyboard" }, { 0xC7, "Home", "Keyboard" },
            { 0xC8, "Up Arrow", "Keyboard" }, { 0xC9, "Page Up", "Keyboard" }, { 0xCB, "Left Arrow", "Keyboard" },
            { 0xCD, "Right Arrow", "Keyboard" }, { 0xCF, "End", "Keyboard" }, { 0xD0, "Down Arrow", "Keyboard" },
            { 0xD1, "Page Down", "Keyboard" }, { 0xD2, "Insert", "Keyboard" }, { 0xD3, "Delete", "Keyboard" },
            { 256, "Left Mouse Button", "Mouse" }, { 257, "Right Mouse Button", "Mouse" }, { 258, "Middle Mouse Button", "Mouse" },
            { 259, "Mouse Button 3", "Mouse" }, { 260, "Mouse Button 4", "Mouse" }, { 261, "Mouse Button 5", "Mouse" },
            { 262, "Mouse Wheel Up", "Mouse" }, { 263, "Mouse Wheel Down", "Mouse" },
            { 264, "Gamepad D-Pad Up", "Gamepad" }, { 265, "Gamepad D-Pad Down", "Gamepad" }, { 266, "Gamepad D-Pad Left", "Gamepad" },
            { 267, "Gamepad D-Pad Right", "Gamepad" }, { 268, "Gamepad Start", "Gamepad" }, { 269, "Gamepad Back", "Gamepad" },
            { 270, "Gamepad L3 (Left Thumb)", "Gamepad" }, { 271, "Gamepad R3 (Right Thumb)", "Gamepad" },
            { 272, "Gamepad LB (Left Bumper)", "Gamepad" }, { 273, "Gamepad RB (Right Bumper)", "Gamepad" },
            { 274, "Gamepad A", "Gamepad" }, { 275, "Gamepad B", "Gamepad" }, { 276, "Gamepad X", "Gamepad" },
            { 277, "Gamepad Y", "Gamepad" }, { 278, "Gamepad LT (Left Trigger)", "Gamepad" }, { 279, "Gamepad RT (Right Trigger)", "Gamepad" }
        };
        return keys;
    }

    std::string GetKeyName(uint32_t a_key) {
        if (a_key == 0) return "None";
        for (const auto& opt : GetKeyOptionsList()) {
            if (opt.code == a_key) return opt.label;
        }
        return std::format("Unknown (0x{:02X})", a_key);
    }

    bool CaseInsensitiveContains(std::string_view haystack, std::string_view needle) {
        if (needle.empty()) return true;
        auto it = std::search(
            haystack.begin(), haystack.end(),
            needle.begin(), needle.end(),
            [](char ch1, char ch2) {
                return std::tolower(static_cast<unsigned char>(ch1)) == std::tolower(static_cast<unsigned char>(ch2));
            }
        );
        return it != haystack.end();
    }

    bool ImGuiSearchableKeyPicker(const char* a_label, uint32_t& a_selectedKeyCode, std::string& a_filterState) {
        bool valueChanged = false;
        std::string currentKeyName = GetKeyName(a_selectedKeyCode);

        if (ImGuiMCP::BeginCombo(a_label, currentKeyName.c_str())) {
            char filterBuffer[64]{};
            std::snprintf(filterBuffer, sizeof(filterBuffer), "%s", a_filterState.c_str());

            ImGuiMCP::SetNextItemWidth(-1.0f);
            if (ImGuiMCP::InputText("##SearchFilter", filterBuffer, sizeof(filterBuffer))) {
                a_filterState = filterBuffer;
            }

            if (ImGuiMCP::IsItemActivated()) {
                ImGuiMCP::SetKeyboardFocusHere(-1);
            }

            ImGuiMCP::Separator();

            if (ImGuiMCP::BeginChild("##KeyOptionsScrollArea", ImGuiMCP::ImVec2{ 0.0f, 180.0f }, true)) {
                for (const auto& option : GetKeyOptionsList()) {
                    if (CaseInsensitiveContains(option.label, a_filterState)) {
                        bool isSelected = (a_selectedKeyCode == option.code);
                        std::string itemDisplayText = std::format("{} [{}]", option.label, option.category);

                        if (ImGuiMCP::Selectable(itemDisplayText.c_str(), isSelected)) {
                            a_selectedKeyCode = option.code;
                            valueChanged = true;
                            a_filterState.clear();
                            ImGuiMCP::CloseCurrentPopup();
                        }

                        if (isSelected) {
                            ImGuiMCP::SetItemDefaultFocus();
                        }
                    }
                }
                ImGuiMCP::EndChild();
            }
            ImGuiMCP::EndCombo();
        }
        return valueChanged;
    }

    void EnsureBackupIniExists() {
        namespace fs = std::filesystem;
        try {
            if (!fs::exists(kBackupIniPath)) {
                if (fs::exists(kIniPath)) {
                    fs::create_directories(fs::path(kBackupIniPath).parent_path());
                    fs::copy_file(kIniPath, kBackupIniPath, fs::copy_options::overwrite_existing);
                    LogMenuFrameworkStatus("Created default backup copy in Backups folder.");
                }
            }
        }
        catch (const std::exception& e) {
            LogMenuFrameworkStatus(std::format("Failed to create backup INI copy: {}", e.what()));
        }
    }

    void SaveSettings(bool a_bEnabledBoot, const Settings& cfg) {
        EnsureBackupIniExists();

        CSimpleIniA ini;
        ini.SetUnicode();
        ini.LoadFile(kIniPath);

        ini.SetBoolValue("Settings", "bEnabled", a_bEnabledBoot, nullptr);
        ini.SetLongValue("Settings", "uToggleKey", cfg.toggleKey);
        ini.SetLongValue("Settings", "uWealthKey", cfg.wealthKey);

        ini.SetLongValue("Settings", "iItemValueMode", cfg.itemValueMode);
        ini.SetLongValue("Settings", "iSpendingOrderMode", cfg.spendingOrderMode);
        ini.SetDoubleValue("Settings", "fBarterModeMultiplier", static_cast<double>(cfg.barterValueMultiplier));
        ini.SetBoolValue("Settings", "bSpendItemsBeforeGold", cfg.spendItemsBeforeGold);
        ini.SetBoolValue("Settings", "bCursedGoldMode", cfg.cursedGoldMode);
        ini.SetBoolValue("Settings", "bIncludePowerModifiers", cfg.includePowerModifiers);
        ini.SetBoolValue("Settings", "bIncludePerkModifiers", cfg.includePerkModifiers);
        ini.SetBoolValue("Settings", "bBlockFollowerInjection", cfg.blockFollowerInjection);
        ini.SetBoolValue("Settings", "bEarlyGoldInjection", cfg.useActivateHandler);

        ini.SetValue("Settings", "sCurrencyName", cfg.currencyName.c_str());
        ini.SetValue("Settings", "sLabelGold", cfg.labelGold.c_str());
        ini.SetValue("Settings", "sLabelGoldAbbr", cfg.labelGoldAbbr.c_str());

        ini.SetValue("Settings", "sColorStatusActive", cfg.colorStatusActive.c_str());
        ini.SetValue("Settings", "sColorStatusInactive", cfg.colorStatusInactive.c_str());
        ini.SetValue("Settings", "sColorGoldAsset", cfg.colorGoldAsset.c_str());
        ini.SetValue("Settings", "sColorLootAsset", cfg.colorLootAsset.c_str());
        ini.SetValue("Settings", "sColorTotalAsset", cfg.colorTotalAsset.c_str());
        ini.SetValue("Settings", "sColorExpectedValue", cfg.colorExpectedValue.c_str());
        ini.SetValue("Settings", "sColorSystemHeader", cfg.colorSystemHeader.c_str());
        ini.SetLongValue("Settings", "iItemNotificationMode", cfg.itemNotificationMode);
        ini.SetBoolValue("Settings", "bShowModeStatus", cfg.showModeStatus);
        ini.SetBoolValue("Settings", "bShowExpectedValue", cfg.showExpectedValue);
        ini.SetBoolValue("Settings", "bEnableDebugNotifications", cfg.enableDebug);

        ini.SetValue("Messages", "sLabelOn", cfg.labelOn.c_str());
        ini.SetValue("Messages", "sLabelOff", cfg.labelOff.c_str());
        ini.SetValue("Messages", "sLabelActive", cfg.labelActive.c_str());
        ini.SetValue("Messages", "sLabelInactive", cfg.labelInactive.c_str());
        ini.SetValue("Messages", "sLabelModeCurrency", cfg.labelModeCurrency.c_str());
        ini.SetValue("Messages", "sLabelModeBarter", cfg.labelModeBarter.c_str());
        ini.SetValue("Messages", "sLabelModeFixed", cfg.labelModeFixed.c_str());

        ini.SaveFile(kIniPath);
    }

    bool ImGuiInputString(const char* label, std::string& strValue) {
        char buffer[256]{};
        std::snprintf(buffer, sizeof(buffer), "%s", strValue.c_str());
        if (ImGuiMCP::InputText(label, buffer, sizeof(buffer))) {
            strValue = buffer;
            return true;
        }
        return false;
    }

    void HexToRGB(const std::string& a_hex, float a_rgb[3]) {
        unsigned int r = 255, g = 255, b = 255;
        if (!a_hex.empty()) {
            const char* hexCStr = (a_hex[0] == '#') ? a_hex.c_str() + 1 : a_hex.c_str();
            uint32_t val = 0;
            auto [ptr, ec] = std::from_chars(hexCStr, hexCStr + std::strlen(hexCStr), val, 16);
            if (ec == std::errc{}) {
                r = (val >> 16) & 0xFF;
                g = (val >> 8) & 0xFF;
                b = val & 0xFF;
            }
        }
        a_rgb[0] = r / 255.0f;
        a_rgb[1] = g / 255.0f;
        a_rgb[2] = b / 255.0f;
    }

    std::string RGBToHex(const float a_rgb[3]) {
        int r = static_cast<int>(std::round(a_rgb[0] * 255.0f));
        int g = static_cast<int>(std::round(a_rgb[1] * 255.0f));
        int b = static_cast<int>(std::round(a_rgb[2] * 255.0f));
        return std::format("{:02X}{:02X}{:02X}", r, g, b);
    }

    bool ImGuiColorHexEdit(const char* label, std::string& hexValue) {
        float rgb[3];
        HexToRGB(hexValue, rgb);

        if (ImGuiMCP::ColorEdit3(label, rgb)) {
            hexValue = RGBToHex(rgb);
            return true;
        }
        return false;
    }

    bool MCPLaceMenu::Register()
    {
        EnsureBackupIniExists();

        if (!SKSEMenuFramework::IsInstalled()) {
            if (auto console = RE::ConsoleLog::GetSingleton()) {
                console->Print("TradeInGems LACE: SKSE Menu Framework not found, in-game menu disabled.");
            }
            LogMenuFrameworkStatus("SKSE Menu Framework was not found. Menu Registration skipped.");
            return false;
        }

        SKSEMenuFramework::SetSection("Trade In Gems NG - LACE");
        auto txt = g_menuTexts.load();
        if (!txt) txt = std::make_shared<MenuTextsMCP>();
        SKSEMenuFramework::AddSectionItem(txt->menuConfiguration.c_str(), RenderMenu);
        SKSEMenuFramework::AddEvent(OnMenuOpen, 0.0f);
        LogMenuFrameworkStatus("LACE MCP Menu successfully registered with SKSE Menu Framework.");
        return true;
    }

    void __stdcall MCPLaceMenu::OnMenuOpen(SKSEMenuFramework::Model::EventType a_event)
    {
        if (a_event == SKSEMenuFramework::Model::EventType::kOpenMenu) {
            ResetUIBufferFromMemory();
        }
    }

    const char* MCPLaceMenu::GetItemValueModeText(const MenuTextsMCP& txt, int mode) {
        switch (mode) {
        case 0:  return txt.itemValueMode0.c_str();
        case 1:  return txt.itemValueMode1.c_str();
        case 2:  return txt.itemValueMode2.c_str();
        default: return "Unknown";
        }
    }

    const char* MCPLaceMenu::GetSpendingPriorityText(const MenuTextsMCP& txt, int mode) {
        switch (mode) {
        case 0:  return txt.spendingPriority0.c_str();
        case 1:  return txt.spendingPriority1.c_str();
        case 2:  return txt.spendingPriority2.c_str();
        case 3:  return txt.spendingPriority3.c_str();
        case 4:  return txt.spendingPriority4.c_str();
        case 5:  return txt.spendingPriority5.c_str();
        case 6:  return txt.spendingPriority6.c_str();
        default: return "None";
        }
    }

    const char* MCPLaceMenu::GetItemNotificationModeText(const MenuTextsMCP& a_txt, int32_t a_mode) {
        switch (a_mode) {
        case 0: return a_txt.itemNotificationMode0.c_str();
        case 1: return a_txt.itemNotificationMode1.c_str();
        case 2: return a_txt.itemNotificationMode2.c_str();
        case 3: return a_txt.itemNotificationMode3.c_str();
        default: return "Unknown";
        }
    }

    void MCPLaceMenu::ResetUIBufferFromMemory()
    {
        CSimpleIniA ini;
        ini.SetUnicode();
        if (ini.LoadFile(kIniPath) >= 0) {
            s_savedBootEnabled = ini.GetBoolValue("Settings", "bEnabled", true);
            s_bEnabledBoot = s_savedBootEnabled;
        }
        if (auto current = g_settings.load()) {
            s_menuSettings = *current;
        }
        s_liveEnabled = g_modEnabled.load();
        s_isDirty = false;
    }

    bool MCPLaceMenu::IsDirty()
    {
        return s_isDirty;
    }

    void MCPLaceMenu::RestoreTabBackup(int activeTab)
    {
        EnsureBackupIniExists();
        CSimpleIniA backupIni;
        backupIni.SetUnicode();

        if (backupIni.LoadFile(kBackupIniPath) < 0) {
            LogMenuFrameworkStatus("Failed to load original backup INI.");
            return;
        }

        if (activeTab == 0)
        {
            s_bEnabledBoot = backupIni.GetBoolValue("Settings", "bEnabled", true);
            s_liveEnabled = backupIni.GetBoolValue("Settings", "bEnabled", true);
            s_menuSettings.toggleKey = static_cast<uint32_t>(backupIni.GetLongValue("Settings", "uToggleKey", 0x0D));
            s_menuSettings.wealthKey = static_cast<uint32_t>(backupIni.GetLongValue("Settings", "uWealthKey", 0x22));
            s_menuSettings.itemValueMode = static_cast<int32_t>(backupIni.GetLongValue("Settings", "iItemValueMode", 0));
            s_menuSettings.spendingOrderMode = static_cast<int32_t>(backupIni.GetLongValue("Settings", "iSpendingOrderMode", 0));
            s_menuSettings.barterValueMultiplier = static_cast<float>(backupIni.GetDoubleValue("Settings", "fBarterModeMultiplier", 1.0));
            s_menuSettings.spendItemsBeforeGold = backupIni.GetBoolValue("Settings", "bSpendItemsBeforeGold", true);
            s_menuSettings.cursedGoldMode = backupIni.GetBoolValue("Settings", "bCursedGoldMode", false);
            s_menuSettings.includePowerModifiers = backupIni.GetBoolValue("Settings", "bIncludePowerModifiers", true);
            s_menuSettings.includePerkModifiers = backupIni.GetBoolValue("Settings", "bIncludePerkModifiers", true);
            s_menuSettings.blockFollowerInjection = backupIni.GetBoolValue("Settings", "bBlockFollowerInjection", true);
            s_menuSettings.useActivateHandler = backupIni.GetBoolValue("Settings", "bEarlyGoldInjection", true);
        }
        else if (activeTab == 1)
        {
            s_menuSettings.colorStatusActive = backupIni.GetValue("Settings", "sColorStatusActive", "00FF00");
            s_menuSettings.colorStatusInactive = backupIni.GetValue("Settings", "sColorStatusInactive", "FF3333");
            s_menuSettings.colorGoldAsset = backupIni.GetValue("Settings", "sColorGoldAsset", "FFD700");
            s_menuSettings.colorLootAsset = backupIni.GetValue("Settings", "sColorLootAsset", "00BFFF");
            s_menuSettings.colorTotalAsset = backupIni.GetValue("Settings", "sColorTotalAsset", "00FF00");
            s_menuSettings.colorExpectedValue = backupIni.GetValue("Settings", "sColorExpectedValue", "FF8C00");
            s_menuSettings.colorSystemHeader = backupIni.GetValue("Settings", "sColorSystemHeader", "A9A9A9");
            s_menuSettings.itemNotificationMode = static_cast<int32_t>(backupIni.GetLongValue("Settings", "iItemNotificationMode", 0));
            s_menuSettings.showModeStatus = backupIni.GetBoolValue("Settings", "bShowModeStatus", true);
            s_menuSettings.showExpectedValue = backupIni.GetBoolValue("Settings", "bShowExpectedValue", false);
            s_menuSettings.enableDebug = backupIni.GetBoolValue("Settings", "bEnableDebugNotifications", false);
        }
        else if (activeTab == 2)
        {
            s_menuSettings.currencyName = backupIni.GetValue("Settings", "sCurrencyName", "Loot");
            s_menuSettings.labelGold = backupIni.GetValue("Settings", "sLabelGold", "Gold");
            s_menuSettings.labelGoldAbbr = backupIni.GetValue("Settings", "sLabelGoldAbbr", "g");

            s_menuSettings.labelOn = backupIni.GetValue("Messages", "sLabelOn", "ON");
            s_menuSettings.labelOff = backupIni.GetValue("Messages", "sLabelOff", "OFF");
            s_menuSettings.labelActive = backupIni.GetValue("Messages", "sLabelActive", "ACTIVE");
            s_menuSettings.labelInactive = backupIni.GetValue("Messages", "sLabelInactive", "INACTIVE");
            s_menuSettings.labelModeCurrency = backupIni.GetValue("Messages", "sLabelModeCurrency", "Currency");
            s_menuSettings.labelModeBarter = backupIni.GetValue("Messages", "sLabelModeBarter", "Barter");
            s_menuSettings.labelModeFixed = backupIni.GetValue("Messages", "sLabelModeFixed", "Fixed");
        }

        LogMenuFrameworkStatus(std::format("Restored original backup settings for tab index {}.", activeTab));
    }

    template <typename Func>
    bool BindUI(Func&& a_imguiFunc) {
        if (a_imguiFunc()) {
            MCPLaceMenu::s_isDirty = true;
            return true;
        }
        return false;
    }

    void MCPLaceMenu::RenderMenu()
    {
        if (!s_bootStateInitialized) {
            ResetUIBufferFromMemory();
            s_bootStateInitialized = true;
            LogMenuFrameworkStatus("Initialized UI menu state buffer.");
        }

        auto txt = g_menuTexts.load();
        if (!txt) txt = std::make_shared<MenuTextsMCP>();

        if (ImGuiMCP::BeginTabBar("TIG-LACE-TabBar"))
        {
            if (ImGuiMCP::BeginTabItem(txt->tabEconomy.c_str()))
            {
                s_activeTab = 0;
                ImGuiMCP::Spacing();

                BindUI([&] { return ImGuiMCP::Checkbox(txt->liveEnabled.c_str(), &s_liveEnabled); }); 
                BindUI([&] { return ImGuiMCP::Checkbox(txt->bootEnabled.c_str(), &s_bEnabledBoot); }); 

                ImGuiMCP::Separator();

                if (ImGuiSearchableKeyPicker(txt->toggleKey.c_str(), s_menuSettings.toggleKey, s_toggleKeySearch)) s_isDirty = true; 
                if (ImGuiSearchableKeyPicker(txt->wealthKey.c_str(), s_menuSettings.wealthKey, s_wealthKeySearch)) s_isDirty = true; 

                ImGuiMCP::Separator();

                BindUI([&] { return ImGuiMCP::SliderInt(txt->itemValueMode.c_str(), &s_menuSettings.itemValueMode, 0, 2, GetItemValueModeText(*txt, s_menuSettings.itemValueMode)); }); 
                BindUI([&] { return ImGuiMCP::SliderInt(txt->spendingOrderMode.c_str(), &s_menuSettings.spendingOrderMode, 0, 6, GetSpendingPriorityText(*txt, s_menuSettings.spendingOrderMode)); });

                if (s_menuSettings.itemValueMode == 1) {
                    BindUI([&] { return ImGuiMCP::SliderFloat(txt->barterValueMultiplier.c_str(), &s_menuSettings.barterValueMultiplier, 0.1f, 5.0f, "%.2f"); }); 
                }

                ImGuiMCP::Separator();

                BindUI([&] { return ImGuiMCP::Checkbox(txt->spendItemsBeforeGold.c_str(), &s_menuSettings.spendItemsBeforeGold); }); 
                BindUI([&] { return ImGuiMCP::Checkbox(txt->cursedGoldMode.c_str(), &s_menuSettings.cursedGoldMode); }); 
                BindUI([&] { return ImGuiMCP::Checkbox(txt->includePowerModifiers.c_str(), &s_menuSettings.includePowerModifiers); }); 
                BindUI([&] { return ImGuiMCP::Checkbox(txt->includePerkModifiers.c_str(), &s_menuSettings.includePerkModifiers); });
                BindUI([&] { return ImGuiMCP::Checkbox(txt->blockFollowerInjection.c_str(), &s_menuSettings.blockFollowerInjection); });
                BindUI([&] { return ImGuiMCP::Checkbox(txt->useActivateHandler.c_str(), &s_menuSettings.useActivateHandler); }); 

                ImGuiMCP::EndTabItem();
            }

            if (ImGuiMCP::BeginTabItem(txt->tabColors.c_str()))
            {
                s_activeTab = 1;
                ImGuiMCP::Spacing();
                ImGuiMCP::TextDisabled(txt->colorsHeader.c_str());
                ImGuiMCP::Spacing();

                if (ImGuiColorHexEdit(txt->colorStatusActive.c_str(), s_menuSettings.colorStatusActive)) s_isDirty = true; 
                if (ImGuiColorHexEdit(txt->colorStatusInactive.c_str(), s_menuSettings.colorStatusInactive)) s_isDirty = true; 
                if (ImGuiColorHexEdit(txt->colorGoldAsset.c_str(), s_menuSettings.colorGoldAsset)) s_isDirty = true; 
                if (ImGuiColorHexEdit(txt->colorLootAsset.c_str(), s_menuSettings.colorLootAsset)) s_isDirty = true; 
                if (ImGuiColorHexEdit(txt->colorTotalAsset.c_str(), s_menuSettings.colorTotalAsset)) s_isDirty = true; 
                if (ImGuiColorHexEdit(txt->colorExpectedValue.c_str(), s_menuSettings.colorExpectedValue)) s_isDirty = true;
                if (ImGuiColorHexEdit(txt->colorSystemHeader.c_str(), s_menuSettings.colorSystemHeader)) s_isDirty = true; 

                ImGuiMCP::Separator();
                BindUI([&] { return ImGuiMCP::SliderInt(txt->itemNotificationMode.c_str(), &s_menuSettings.itemNotificationMode, 0, 3, GetItemNotificationModeText(*txt, s_menuSettings.itemNotificationMode)); }); 
                BindUI([&] { return ImGuiMCP::Checkbox(txt->showModeStatus.c_str(), &s_menuSettings.showModeStatus); }); 
                BindUI([&] { return ImGuiMCP::Checkbox(txt->showExpectedValue.c_str(), &s_menuSettings.showExpectedValue); }); 
                BindUI([&] { return ImGuiMCP::Checkbox(txt->enableDebug.c_str(), &s_menuSettings.enableDebug); }); 

                ImGuiMCP::EndTabItem();
            }

            if (ImGuiMCP::BeginTabItem(txt->tabLabels.c_str()))
            {
                s_activeTab = 2;
                ImGuiMCP::Spacing();
                ImGuiMCP::TextDisabled(txt->labelsHeader.c_str());
                ImGuiMCP::Spacing();

                if (ImGuiInputString(txt->currencyName.c_str(), s_menuSettings.currencyName)) s_isDirty = true; 
                if (ImGuiInputString(txt->labelGold.c_str(), s_menuSettings.labelGold)) s_isDirty = true; 
                if (ImGuiInputString(txt->labelGoldAbbr.c_str(), s_menuSettings.labelGoldAbbr)) s_isDirty = true;

                ImGuiMCP::Separator();

                if (ImGuiInputString(txt->labelOn.c_str(), s_menuSettings.labelOn)) s_isDirty = true; 
                if (ImGuiInputString(txt->labelOff.c_str(), s_menuSettings.labelOff)) s_isDirty = true;
                if (ImGuiInputString(txt->labelActive.c_str(), s_menuSettings.labelActive)) s_isDirty = true;
                if (ImGuiInputString(txt->labelInactive.c_str(), s_menuSettings.labelInactive)) s_isDirty = true; 

                ImGuiMCP::Separator();

                if (ImGuiInputString(txt->labelModeCurrency.c_str(), s_menuSettings.labelModeCurrency)) s_isDirty = true; 
                if (ImGuiInputString(txt->labelModeBarter.c_str(), s_menuSettings.labelModeBarter)) s_isDirty = true; 
                if (ImGuiInputString(txt->labelModeFixed.c_str(), s_menuSettings.labelModeFixed)) s_isDirty = true; 

                ImGuiMCP::EndTabItem();
            }

            ImGuiMCP::EndTabBar();
        }

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::Spacing();

        if (IsDirty()) {
            ImGuiMCP::TextColored({ 1.0f, 0.8f, 0.2f, 1.0f }, txt->unsavedPending.c_str());
            ImGuiMCP::Spacing();
        }

        if (ImGuiMCP::Button(txt->btnApply.c_str()))
        {
            SaveSettings(s_bEnabledBoot, s_menuSettings);
            g_modEnabled.store(s_liveEnabled);
            s_savedBootEnabled = s_bEnabledBoot;

            TradeInGems::LoadSettings();
            TradeInGems::LoadMenuTexts();

            s_isDirty = false; 
            LogMenuFrameworkStatus("Settings saved to INI and reloaded via LoadSettings() and LoadMenuTexts().");
            s_savedTimer = 3.0f;
            s_confirmRestoreTab = false;
        }

        ImGuiMCP::SameLine();

        if (ImGuiMCP::Button(txt->btnDiscard.c_str())) {
            ResetUIBufferFromMemory();
            s_confirmRestoreTab = false;
            LogMenuFrameworkStatus("UI state buffer reset to active runtime settings.");
        }

        ImGuiMCP::SameLine();

        if (!s_confirmRestoreTab) {
            if (ImGuiMCP::Button(txt->btnRestore.c_str())) {
                s_confirmRestoreTab = true;
            }
        }
        else {
            ImGuiMCP::TextColored({ 1.0f, 0.4f, 0.4f, 1.0f }, txt->confirmRestore.c_str());
            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button(txt->btnYesRestore.c_str())) {
                RestoreTabBackup(s_activeTab);
                s_isDirty = true; 
                s_confirmRestoreTab = false;
            }
            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button(txt->btnCancel.c_str())) {
                s_confirmRestoreTab = false;
            }
        }

        if (s_savedTimer > 0.0f) {
            ImGuiMCP::SameLine();
            ImGuiMCP::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, txt->savedSuccess.c_str());
            s_savedTimer -= ImGuiMCP::GetIO()->DeltaTime;
        }
    }
}