#pragma once

#include <cstdint>

namespace TradeInGems {
    inline std::atomic<bool> g_modEnabled{ true };
    template <typename... Args>
    std::string SafeFormat(const std::string& userTemplate, const std::string& defaultTemplate, Args&&... args) {
        try {
            return std::vformat(userTemplate, std::make_format_args(args...));
        }
        catch (...) {
            return std::vformat(defaultTemplate, std::make_format_args(args...));
        }
    }
    // we have 3 different blocks for default message templates that appear in different scenarios - this one is for the safe formatting function when bad formatting is detected
	// they should never appear if the mod is properly configured - they can be adjusted to be different or have indicators for debug, but I left them all the same for now
    namespace Defaults {
        inline const std::string msgCursedGold = "<font color='#{}'>Cursed </font><font color='#{}'>{}</font><font color='#{}'> burns your flesh!</font>";
        inline const std::string msgTradedPerItem = "<font color='#{}'>Traded {}x </font><font color='#{}'>{}</font>";
        inline const std::string msgTruncatedHeader = "<font color='#{}'>Traded: </font>{}";
        inline const std::string msgTruncatedMore = " <font color='#{}'>and {} more...</font>";
        inline const std::string msgMsgBoxHeader = "{} Traded";
        inline const std::string msgMsgBoxLimiter = "________________________________";
        inline const std::string msgMsgBoxTruncated = "...and {} more types of {}.";
        inline const std::string msgTradingStatusMsg = "<font color='#{}'>Cannot revalue your </font><font color='#{}'>{}</font><font color='#{}'> stash while occupied.</font>";
        inline const std::string msgTradingStatusToggle = "<font color='#{}'>{}</font><font color='#{}'> Trading: </font>{}";
        inline const std::string msgAssetSummary = "<font color='#{}'>{}: {}</font> <font color='#{}'>|</font> <font color='#{}'>{}: {}</font> <font color='#{}'>|</font> <font color='#{}'>Total: {}</font>";
        inline const std::string msgExpectedValue = "<font color='#{}'>Expected {} Value: {}%</font>";
        inline const std::string msgFrameworkStatus = "<font color='#{}'>{} Trading: {} </font><font color='#{}'>| Mode: {}, </font><font color='#{}'>{}</font><font color='#{}'> 1st | Speech {}</font>";
        inline const std::string msgChangeReceived = "<font color='#{}'>Change Received: </font><font color='#{}'>{}{}</font><font color='#{}'>. Wasted </font><font color='#{}'>{}{}</font>.";
        inline const std::string msgNoChange = "<font color='#{}'>No Change Received. Wasted </font><font color='#{}'>{}{}</font>.";
    }

    enum class ItemValueMode : int32_t {
        kBuy = 0,       
        kSell = 1, 
        kBaseValue = 2, 
    };

    enum class ItemNotificationMode : int32_t {
        kPerItem = 0,          
        kTruncatedSummary = 1, 
        kMessageBoxPopup = 2,  
        kConsoleLogOnly = 3    
    };

    enum class SpendingOrderMode : int32_t {
        kWorstWeightToValue = 0, // default
        kBestWeightToValue = 1,
        kHeaviestFirst = 2,
        kLightestFirst = 3,
        kCheapestFirst = 4,
        kMostExpensiveFirst = 5,
        kNoSorting = 6
    };

	// defaults struct, only missing bool modEnabled{true} setting on purpose
    struct Settings {
        // Keybinds & States
        uint32_t toggleKey{ 0x0D };
        uint32_t wealthKey{ 0x22 };

        // Economy Settings
        int32_t itemValueMode{ static_cast<int32_t>(ItemValueMode::kBuy) };
        int32_t spendingOrderMode{ static_cast<int32_t>(SpendingOrderMode::kWorstWeightToValue) };
        float barterValueMultiplier{ 1.0f };
        bool spendItemsBeforeGold{ true };
        bool cursedGoldMode{ false };
        bool includePowerModifiers{ true };
        bool includePerkModifiers{ true };
        bool blockFollowerInjection{ true };
        bool useActivateHandler{ true };
        RE::FormID goldFormID{ 0x0000000F };

        std::string currencyName{ "Loot" };
        std::string labelGold{ "Gold" };
        std::string labelGoldAbbr{ "g" };

        // colors and display settings
        std::string colorStatusActive{ "00FF00" };
        std::string colorStatusInactive{ "FF3333" };
        std::string colorGoldAsset{ "FFD700" };
        std::string colorLootAsset{ "00BFFF" };
        std::string colorTotalAsset{ "00FF00" };
        std::string colorExpectedValue{ "FF8C00" };
        std::string colorSystemHeader{ "A9A9A9" };

        int32_t itemNotificationMode{ 0 };

        bool showModeStatus{ true };
        bool showExpectedValue{ false };
        bool enableDebug{ false };


        // status and mode labels
        std::string labelOn{ "ON" };
        std::string labelOff{ "OFF" };
        std::string labelActive{ "ACTIVE" };
        std::string labelInactive{ "INACTIVE" };
        std::string labelModeCurrency{ "Currency" };
        std::string labelModeBarter{ "Barter" };
        std::string labelModeFixed{ "Fixed" };

        // we have 3 different blocks for default message templates that appear in different scenarios - this one is for initialization if .ini is missing
        std::string msgCursedGold{ "<font color='#{}'>Cursed </font><font color='#{}'>{}</font><font color='#{}'> burns your flesh!</font>" };
        std::string msgTradedPerItem{ "<font color='#{}'>Traded {}x </font><font color='#{}'>{}</font>" };
        std::string msgTruncatedHeader{ "<font color='#{}'>Traded: </font>{}" };
        std::string msgTruncatedMore{ " <font color='#{}'>and {} more...</font>" };
        std::string msgMsgBoxHeader{ "{} Traded" };
        std::string msgMsgBoxLimiter{ "________________________________" }; // font mods change how effective this preset box limiter is
        std::string msgMsgBoxTruncated{ "...and {} more types of {}." };
        std::string msgTradingStatusMsg{ "<font color='#{}'>Cannot revalue your </font><font color='#{}'>{}</font><font color='#{}'> stash while occupied.</font>" };
        std::string msgTradingStatusToggle{ "<font color='#{}'>{}</font><font color='#{}'> Trading: </font>{}" };
        std::string msgAssetSummary{ "<font color='#{}'>{}: {}</font> <font color='#{}'>|</font> <font color='#{}'>{}: {}</font> <font color='#{}'>|</font> <font color='#{}'>Total: {}</font>" }; 
        std::string msgExpectedValue{ "<font color='#{}'>Expected {} Value: {}%</font>" };
        std::string msgFrameworkStatus{ "<font color='#{}'>{} Trading: {} </font><font color='#{}'>| Mode: {}, </font><font color='#{}'>{}</font><font color='#{}'> 1st | Speech {}</font>" };
        std::string msgChangeReceived{ "<font color='#{}'>Change Received: </font><font color='#{}'>{}{}</font><font color='#{}'>. Wasted </font><font color='#{}'>{}{}</font>." }; 
        std::string msgNoChange{ "<font color='#{}'>No Change Received. Wasted </font><font color='#{}'>{}{}</font>." };

    };

    inline std::atomic<std::shared_ptr<const Settings>> g_settings{std::make_shared<Settings>()};

    void LoadSettings();
    void InitializeFirstLoadSettings();
}
