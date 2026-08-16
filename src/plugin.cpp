#include "log.h"
#include "gemRegistry.h"
#include "settings.h"
#include "UINotifications.h"
#include "ItemSorter.h"
#include "MCPlaceMenu.h"
#include <algorithm>
#include <atomic>
#include <cctype>      
#include <format>
#include <fstream>      
#include <map>
#include <sstream>      
#include <string>
#include <unordered_map>
#include <vector>
#include <RE/S/Script.h>
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <SimpleIni.h>

namespace TradeInGems
{

    std::unordered_map<RE::FormID, int32_t> ItemValues = {

    };

    std::atomic<int32_t> g_ghostGoldAdded{ 0 };
    std::atomic<int32_t> g_realGoldBeforeTransaction{ 0 };
    RE::ObjectRefHandle g_capturedMerchantHandle{};
    std::atomic<bool> g_isRunningSettle{ false };

    // this function loads when you boot or start new games
    void InitializeFirstLoadSettings() {
        CSimpleIniA ini;
        ini.SetUnicode();
        const auto path = L"Data/SKSE/Plugins/TradeInGemsLACE/TradeInGemsLACE.ini";
        if (ini.LoadFile(path) >= 0) {
            g_modEnabled.store(ini.GetBoolValue("Settings", "bEnabled", true));
        }
    }

    void LoadSettings() {
        CSimpleIniA ini;
        ini.SetUnicode();
        const auto path = L"Data/SKSE/Plugins/TradeInGemsLACE/TradeInGemsLACE.ini";

        if (ini.LoadFile(path) >= 0) {
            auto newSettings = std::make_shared<Settings>();

            newSettings->toggleKey = static_cast<uint32_t>(ini.GetLongValue("Settings", "uToggleKey", 0x0D));
            newSettings->wealthKey = static_cast<uint32_t>(ini.GetLongValue("Settings", "uWealthKey", 0x22));

            newSettings->itemValueMode = static_cast<int32_t>(ini.GetLongValue("Settings", "iItemValueMode", 0));
            newSettings->spendingOrderMode = static_cast<int32_t>(ini.GetLongValue("Settings", "iSpendingOrderMode", static_cast<int32_t>(SpendingOrderMode::kWorstWeightToValue)));
            newSettings->barterValueMultiplier = static_cast<float>(ini.GetDoubleValue("Settings", "fBarterModeMultiplier", 1.0));
            newSettings->spendItemsBeforeGold = ini.GetBoolValue("Settings", "bSpendItemsBeforeGold", true);
            newSettings->cursedGoldMode = ini.GetBoolValue("Settings", "bCursedGoldMode", false);
            newSettings->includePowerModifiers = ini.GetBoolValue("Settings", "bIncludePowerModifiers", true);
            newSettings->includePerkModifiers = ini.GetBoolValue("Settings", "bIncludePerkModifiers", true);
            newSettings->blockFollowerInjection = ini.GetBoolValue("Settings", "bBlockFollowerInjection", true);
            newSettings->useActivateHandler = ini.GetBoolValue("Settings", "bEarlyGoldInjection", true);
            const char* goldStr = ini.GetValue("Settings", "sGoldFormID", "0x0000000F");
            try {
                newSettings->goldFormID = static_cast<RE::FormID>(std::stoul(goldStr, nullptr, 16));
            }
            catch (...) {
                newSettings->goldFormID = 0x0000000F;
            }

            newSettings->colorStatusActive = ini.GetValue("Settings", "sColorStatusActive", "00FF00");
            newSettings->colorStatusInactive = ini.GetValue("Settings", "sColorStatusInactive", "FF3333");
            newSettings->colorGoldAsset = ini.GetValue("Settings", "sColorGoldAsset", "FFD700");
            newSettings->colorLootAsset = ini.GetValue("Settings", "sColorLootAsset", "00BFFF");
            newSettings->colorTotalAsset = ini.GetValue("Settings", "sColorTotalAsset", "00FF00");
            newSettings->colorExpectedValue = ini.GetValue("Settings", "sColorExpectedValue", "FF8C00");
            newSettings->colorSystemHeader = ini.GetValue("Settings", "sColorSystemHeader", "A9A9A9");
            newSettings->showModeStatus = ini.GetBoolValue("Settings", "bShowModeStatus", true);
            newSettings->showExpectedValue = ini.GetBoolValue("Settings", "bShowExpectedValue", false);
            newSettings->itemNotificationMode = static_cast<int32_t>(ini.GetLongValue("Settings", "iItemNotificationMode", 0));
            newSettings->enableDebug = ini.GetBoolValue("Settings", "bEnableDebugNotifications", false);

            newSettings->labelOn = ini.GetValue("Messages", "sLabelOn", "ON");
            newSettings->labelOff = ini.GetValue("Messages", "sLabelOff", "OFF");
            newSettings->labelActive = ini.GetValue("Messages", "sLabelActive", "ACTIVE");
            newSettings->labelInactive = ini.GetValue("Messages", "sLabelInactive", "INACTIVE");
            newSettings->labelModeCurrency = ini.GetValue("Messages", "sLabelModeCurrency", "Currency");
            newSettings->labelModeBarter = ini.GetValue("Messages", "sLabelModeBarter", "Barter");
            newSettings->labelModeFixed = ini.GetValue("Messages", "sLabelModeFixed", "Fixed");

            newSettings->currencyName = ini.GetValue("Settings", "sCurrencyName", "Loot");
            newSettings->labelGold = ini.GetValue("Settings", "sLabelGold", "Gold");
            newSettings->labelGoldAbbr = ini.GetValue("Settings", "sLabelGoldAbbr", "g");

            // we have 3 different blocks for default message templates that appear in different scenarios - this one provides fallbacks if .ini has missing entries
            newSettings->msgCursedGold = ini.GetValue("Messages", "sMsgCursedGold", "<font color='#{}'>Cursed </font><font color='#{}'>{}</font><font color='#{}'> burns your flesh!</font>");
            newSettings->msgTradedPerItem = ini.GetValue("Messages", "sMsgTradedPerItem", "<font color='#{}'>Traded {}x </font><font color='#{}'>{}</font>");
            newSettings->msgTruncatedHeader = ini.GetValue("Messages", "sMsgTruncatedHeader", "<font color='#{}'>Traded: </font>{}");
            newSettings->msgTruncatedMore = ini.GetValue("Messages", "sMsgTruncatedMore", " <font color='#{}'>and {} more...</font>");
            newSettings->msgMsgBoxHeader = ini.GetValue("Messages", "sMsgMsgBoxHeader", "{} Traded");
            newSettings->msgMsgBoxLimiter = ini.GetValue("Messages", "sMsgMsgBoxLimiter", "________________________________");
            newSettings->msgMsgBoxTruncated = ini.GetValue("Messages", "sMsgMsgBoxTruncated", "...and {} more types of {}.");
            newSettings->msgTradingStatusMsg = ini.GetValue("Messages", "sMsgTradingStatusMsg", "<font color='#{}'>Cannot revalue your </font><font color='#{}'>{}</font><font color='#{}'> stash while occupied.</font>");
            newSettings->msgTradingStatusToggle = ini.GetValue("Messages", "sMsgTradingStatusToggle", "<font color='#{}'>{}</font><font color='#{}'> Trading: </font>{}");
            newSettings->msgAssetSummary = ini.GetValue("Messages", "sMsgAssetSummary", "<font color='#{}'>{}: {}</font> <font color='#{}'>|</font> <font color='#{}'>{}: {}</font> <font color='#{}'>|</font> <font color='#{}'>Total: {}</font>");
            newSettings->msgExpectedValue = ini.GetValue("Messages", "sMsgExpectedValue", "<font color='#{}'>Expected {} Value: {}%</font>");
            newSettings->msgFrameworkStatus = ini.GetValue("Messages", "sMsgFrameworkStatus", "<font color='#{}'>{} Trading: {} </font><font color='#{}'>| Mode: {}, </font><font color='#{}'>{}</font><font color='#{}'> 1st | Speech {}</font>");
            newSettings->msgChangeReceived = ini.GetValue("Messages", "sMsgChangeReceived", "<font color='#{}'>Change Received: </font><font color='#{}'>{}{}</font><font color='#{}'>. Wasted </font><font color='#{}'>{}{}</font>.");
            newSettings->msgNoChange = ini.GetValue("Messages", "sMsgNoChange", "<font color='#{}'>No Change Received. Wasted </font><font color='#{}'>{}{}</font>.");

            g_settings.store(newSettings);
        }
    }

    // Using po3's method to obtain editorID from formID (required for the console command lace add if you don't type the param)
    std::string GetEditorIDFromFormID(RE::FormID a_formID)
    {
        if (a_formID == 0) {
            return "";
        }

        using _GetFormEditorID = const char* (*)(std::uint32_t);

        static auto tweaks = REX::W32::GetModuleHandleW(L"po3_Tweaks.dll");
        if (tweaks) {
            static auto func = reinterpret_cast<_GetFormEditorID>(
                REX::W32::GetProcAddress(tweaks, "GetFormEditorID")
                );
            if (func) {
                if (const char* edid = func(a_formID)) {
                    return edid; 
                }
            }
        }

        return ""; 
    }

    bool IsFollower(RE::TESObjectREFR* a_ref) {
        if (!a_ref) return false;
        auto actor = a_ref->As<RE::Actor>();
        if (!actor) return false;

        if (actor->IsPlayerTeammate()) return true;

        static RE::TESFaction* currentFollowerFaction = nullptr;
        if (!currentFollowerFaction) {
            //0x0005C84D is for potential followers, 0x0005C84E is for current followers, using the first would block mercenary hiring 
			currentFollowerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);
        }
        if (currentFollowerFaction && actor->IsInFaction(currentFollowerFaction)) return true;

        return false;
    }

    struct EconomyCache {
        float fBarterMax = 3.3f;
        float fBarterMin = 2.0f;

        void Refresh() {
            auto gsc = RE::GameSettingCollection::GetSingleton();
            if (gsc) {
                if (auto settingMax = gsc->GetSetting("fBarterMax")) fBarterMax = settingMax->GetFloat();
                if (auto settingMin = gsc->GetSetting("fBarterMin")) fBarterMin = settingMin->GetFloat();
            }
        }
    };
    EconomyCache g_ecoCache;

    float ApplyPerkPriceModifier(float inputVal_p, bool isBuyPrice, RE::Actor* a_merchant = nullptr) {
        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return inputVal_p;

        RE::Actor* merchantActor = a_merchant;
        if (!merchantActor) {
            // settle() will be blind to this, as we reset this before running it, but inject needs it if pass fails
            if (auto capturedSmartPtr = g_capturedMerchantHandle.get()) {
                merchantActor = capturedSmartPtr->As<RE::Actor>();
            }
        }

        float modifiedPrice = inputVal_p;
        auto entryPoint = isBuyPrice ? RE::BGSEntryPoint::ENTRY_POINT::kModBuyPrices : RE::BGSEntryPoint::ENTRY_POINT::kModSellPrices;

        RE::BGSEntryPoint::HandleEntryPoint(entryPoint, player, merchantActor, std::addressof(modifiedPrice));

        return modifiedPrice;
    }

    float GetMerchantBarterMultiplier(bool a_isBuyPrice, RE::Actor* a_merchant = nullptr) {
        auto cfg = g_settings.load();
        if (cfg->itemValueMode == static_cast<int32_t>(ItemValueMode::kBaseValue)) return 1.0f; // remember to change this if we ever want to apply a modifier to fixed mode

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return 1.0f;

        float speechSkill = player->AsActorValueOwner() ? player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeech) : 0.0f;

        float perkMod = 1.0f;
        if (cfg->includePerkModifiers) {
            perkMod = ApplyPerkPriceModifier(1.0f, a_isBuyPrice, a_merchant);
        }

        float powerMod = 1.0f;
        if (cfg->includePowerModifiers) {
            if (auto avOwner = player->AsActorValueOwner()) {
                speechSkill += avOwner->GetActorValue(RE::ActorValue::kSpeechcraftModifier);
                float rawPower = avOwner->GetActorValue(RE::ActorValue::kSpeechcraftPowerModifier) / 100.0f;
                powerMod = a_isBuyPrice ? std::max(0.0f, 1.0f - rawPower) : (1.0f + rawPower);
            }
        }

        speechSkill = std::clamp(speechSkill, 0.0f, 100.0f);
        float progress = speechSkill / 100.0f;
        float barterFactor = g_ecoCache.fBarterMax - ((g_ecoCache.fBarterMax - g_ecoCache.fBarterMin) * progress);
        barterFactor = std::clamp(barterFactor, 1.0f, g_ecoCache.fBarterMax);

        float barterMult = perkMod * powerMod;
        if (a_isBuyPrice) {
            return barterMult * barterFactor;
        }
        else {
            float customMult = std::max(cfg->barterValueMultiplier, 0.0f);
            return (barterMult * customMult) / barterFactor;
        }
    }

    int32_t GetTotalGemWealthFast(RE::Actor* a_merchant) {
        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return 0;

        bool isBuy = (g_settings.load()->itemValueMode == static_cast<int32_t>(ItemValueMode::kBuy));
        float multiplier = GetMerchantBarterMultiplier(isBuy, a_merchant);

        int32_t totalWealth = 0;
        for (auto& [item, data] : player->GetInventory()) {
            if (item && ItemValues.contains(item->GetFormID())) {
                float rawValue = static_cast<float>(item->GetGoldValue());
                int32_t unitValue = isBuy ? static_cast<int32_t>(std::lround(std::max(rawValue * 1.05f, rawValue * multiplier))) : static_cast<int32_t>(std::lround(std::min(rawValue * multiplier, rawValue)));
                totalWealth += (unitValue * data.first);
            }
        }
        return totalWealth;
    }
    
    void SetGemsHidden(bool a_hide) {
        for (auto const& [formID, val] : ItemValues) {
            auto gem = RE::TESForm::LookupByID<RE::TESForm>(formID);
            if (gem) {
                if (a_hide) gem->formFlags |= 0x00000004;
                else gem->formFlags &= ~0x00000004;
            }
        }
    }

    void Inject(RE::TESObjectREFR* a_merchant = nullptr) {
        auto cfg = g_settings.load();
        bool modEnabled = g_modEnabled.load();
        if ((!modEnabled && !cfg->cursedGoldMode) || g_ghostGoldAdded.load() > 0) return;

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        RE::Actor* merchantActor = a_merchant ? a_merchant->As<RE::Actor>() : nullptr;
        if (!merchantActor) {
            if (auto smartPtr = g_capturedMerchantHandle.get()) {
                merchantActor = smartPtr->As<RE::Actor>();
            }
        }
        if (cfg->blockFollowerInjection && IsFollower(merchantActor)) return;
        int32_t wealth = GetTotalGemWealthFast(merchantActor);

        auto goldForm = RE::TESForm::LookupByID<RE::TESBoundObject>(cfg->goldFormID);
        if (player && wealth > 0 && goldForm) {
            int32_t realGoldNow = player->GetItemCount(goldForm);
            g_realGoldBeforeTransaction.store(realGoldNow);
            if (modEnabled) {
                SetGemsHidden(true);
                player->As<RE::TESObjectREFR>()->AddObjectToContainer(goldForm, nullptr, wealth, nullptr);

                g_ghostGoldAdded.store(wealth);
            }
        }
        if (cfg->enableDebug) {
            std::string msg = std::format("[DEBUG] Inject Done | Ghost Gold: {} | Real Gold: {}", TradeInGems::g_ghostGoldAdded.load(), TradeInGems::g_realGoldBeforeTransaction.load());
            RE::DebugNotification(msg.c_str());
            if (auto console = RE::ConsoleLog::GetSingleton()) console->Print(msg.c_str());
        }
    }

    void Settle(RE::TESObjectREFR* a_capturedMerchant = nullptr) {
        auto cfg = g_settings.load();
        bool modEnabled = g_modEnabled.load();
        bool expected = false;
        if (!g_isRunningSettle.compare_exchange_strong(expected, true)) {
            return;
        }
        struct SettleGuard {
            ~SettleGuard() { g_isRunningSettle.store(false); }
        } guard;

        SetGemsHidden(false);

        if (cfg->enableDebug) {
            std::string name = a_capturedMerchant ? a_capturedMerchant->GetDisplayFullName() : "Unknown";
            std::string msg = std::format("[DEBUG] Settle Entered | Merchant: {} (0x{:X})", name, a_capturedMerchant ? a_capturedMerchant->GetFormID() : 0);
            RE::DebugNotification(msg.c_str());
            if (auto console = RE::ConsoleLog::GetSingleton()) console->Print(msg.c_str());
        }

        if ((!modEnabled && !cfg->cursedGoldMode) && g_ghostGoldAdded.load() <= 0) return;
        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        auto goldForm = RE::TESForm::LookupByID<RE::TESBoundObject>(cfg->goldFormID);
        if (!goldForm) return;

        int32_t currentGold = player->GetItemCount(goldForm);
        int32_t ghost = g_ghostGoldAdded.load();
        g_ghostGoldAdded.store(0);

        int32_t originalRealGold = g_realGoldBeforeTransaction.load();
        g_realGoldBeforeTransaction.store(0);

        if (cfg->cursedGoldMode && originalRealGold > 0) {
            int32_t totalSpent = (originalRealGold + ghost) - currentGold;
            int32_t realGoldExpenses = 0;

            if (totalSpent > 0) {
                if (!modEnabled || ghost == 0) {
                    realGoldExpenses = totalSpent;
                }
                else if (cfg->spendItemsBeforeGold) {
                    realGoldExpenses = (totalSpent > ghost) ? (totalSpent - ghost) : 0;
                }
                else {
                    realGoldExpenses = std::min(originalRealGold, totalSpent);
                }
            }

            if (realGoldExpenses > 0) {
                if (auto avOwner = player->AsActorValueOwner()) {
                    float damageAmount = static_cast<float>(realGoldExpenses);
                    avOwner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -damageAmount);
                }
                RE::DebugNotification(TradeInGems::SafeFormat(cfg->msgCursedGold, TradeInGems::Defaults::msgCursedGold,
                    cfg->colorStatusInactive, cfg->colorGoldAsset, cfg->labelGold, cfg->colorStatusInactive).c_str());
            }
        }

        if (ghost <= 0) return; 
        RE::TESObjectREFR* merchantRef = a_capturedMerchant;
        if (!merchantRef) {
            if (auto topicManager = RE::MenuTopicManager::GetSingleton()) {
                auto handle = topicManager->speaker ? topicManager->speaker : topicManager->lastSpeaker;
                if (auto smartPtr = handle.get()) {
                    merchantRef = smartPtr.get();
                }
            }
        }
        RE::Actor* merchantActor = a_capturedMerchant ? merchantRef->As<RE::Actor>() : nullptr;
        RE::TESObjectREFR* merchantChest = nullptr;

        if (merchantActor) {
            auto GetFactionMerchantChest = [](RE::TESFaction* a_faction) -> RE::TESObjectREFR* {
                return (a_faction && a_faction->vendorData.merchantContainer) ?
                    a_faction->vendorData.merchantContainer : nullptr;
            };

            if (auto extraFactionChanges = merchantActor->extraList.GetByType<RE::ExtraFactionChanges>()) {
                for (const auto& change : extraFactionChanges->factionChanges) {
                    if (auto chest = GetFactionMerchantChest(change.faction)) {
                        merchantChest = chest;
                        break;
                    }
                }
            }

            if (!merchantChest) {
                if (auto baseNPC = merchantActor->GetActorBase()) {
                    for (const auto& factionInfo : baseNPC->factions) {
                        if (auto chest = GetFactionMerchantChest(factionInfo.faction)) {
                            merchantChest = chest;
                            break;
                        }
                    }
                }
            }
        }
        RE::TESObjectREFR* targetContainer = merchantChest ? merchantChest : merchantRef;

        if (cfg->enableDebug && targetContainer) {
            std::string sourceLabel = merchantChest ? "Linked Chest" : (merchantRef ? "Target's Pockets" : "Invalid Context");
            std::string containerName = targetContainer->GetDisplayFullName();
            if (containerName.empty()) containerName = "Unnamed";

            std::string msg = std::format("[DEBUG] Container Found via {} | Name: {} | FormID: 0x{:X}", sourceLabel, containerName, targetContainer->GetFormID());
            RE::DebugNotification(msg.c_str());
            if (auto console = RE::ConsoleLog::GetSingleton()) console->Print(msg.c_str());
        }

        int32_t goldToTakeBack = 0;
        int32_t debt = 0;

        if (cfg->spendItemsBeforeGold) {
            int32_t goldSpentFromPool = originalRealGold + ghost - currentGold;

            if (goldSpentFromPool <= 0) {
                goldToTakeBack = ghost;
                debt = 0;
            }
            else {
                debt = std::min(ghost, goldSpentFromPool); 

                int32_t ghostRemaining = ghost - goldSpentFromPool;
                if (ghostRemaining > 0) {
                    goldToTakeBack = ghostRemaining;
                }
                else {
                    goldToTakeBack = 0;
                }
            }
        }
        else {
            goldToTakeBack = std::min(currentGold, ghost);
            debt = (currentGold < ghost) ? (ghost - currentGold) : 0;
        }

        int32_t totalGemCurrencyValue = 0;

        if (goldToTakeBack > 0) {
            player->RemoveItem(goldForm, goldToTakeBack, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
        }

        if (debt > 0) {
            int32_t remainingDebt = debt;
            float totalValueTraded = 0.0f; 
            auto inv = player->GetInventory();
            std::vector<std::pair<std::string, int32_t>> removedTotals;
            auto addRemovedTotal = [&](const std::string& name, int32_t count) {
                for (auto& [itemName, itemTotal] : removedTotals) {
                    if (itemName == name) {
                        itemTotal += count;
                        return;
                    }
                }
                removedTotals.push_back({ name, count });
            };

            std::vector<TradeInGems::SortableItem> sortedItems;
            for (auto& [item, data] : inv) {
                if (item && ItemValues.contains(item->GetFormID())) {
                    sortedItems.push_back({item, data.first, static_cast<float>(item->GetGoldValue()), item->GetWeight()});
                }
            }
            TradeInGems::SortItemsForSpending(sortedItems, cfg->spendingOrderMode);
            
            bool isBuyMode = (cfg->itemValueMode == static_cast<int32_t>(ItemValueMode::kBuy));
            float merchantMultiplier = GetMerchantBarterMultiplier(isBuyMode, merchantActor);

            for (auto& itemData : sortedItems) {
                if (remainingDebt <= 0) break;

                int32_t unitValue = static_cast<int32_t>(std::lround(isBuyMode ? std::max(itemData.baseValue * 1.05f, itemData.baseValue * merchantMultiplier)
                    : std::min(itemData.baseValue * merchantMultiplier, itemData.baseValue)));
                if (unitValue <= 0) continue;

                int32_t stackTradeValue = unitValue * itemData.count;

                int32_t itemsToRemove = 0;
                int32_t subStackTradeValue = 0;

                if (remainingDebt >= stackTradeValue) {
                    itemsToRemove = itemData.count;
                    subStackTradeValue = stackTradeValue;
                }
                else {
                    itemsToRemove = (remainingDebt + unitValue - 1) / unitValue;
                    subStackTradeValue = itemsToRemove * unitValue;
                }
                player->RemoveItem(itemData.item, itemsToRemove, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                if (targetContainer) targetContainer->AddObjectToContainer(itemData.item, nullptr, itemsToRemove, nullptr);

                remainingDebt -= subStackTradeValue;
                totalValueTraded += (itemData.baseValue * itemsToRemove);
                totalGemCurrencyValue += subStackTradeValue;
                addRemovedTotal(itemData.item->GetName(), itemsToRemove);
            }

            if (totalValueTraded > 0.0f) {
                player->AddSkillExperience(RE::ActorValue::kSpeech, totalValueTraded);
            }

            TradeInGems::UI::DispatchItemNotification(static_cast<TradeInGems::ItemNotificationMode>(cfg->itemNotificationMode),removedTotals,cfg);

            int32_t ghostToRemoveFromMerchant = totalGemCurrencyValue;
            if (remainingDebt < 0) {
                ghostToRemoveFromMerchant -= std::abs(remainingDebt);
            }
            if (targetContainer && ghostToRemoveFromMerchant > 0) {
                targetContainer->RemoveItem(goldForm, ghostToRemoveFromMerchant, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            }

            if (remainingDebt < 0) {               
                if (targetContainer) {
                    int32_t changeOwed = std::abs(remainingDebt);
                    int32_t pocketChange = 0;
                    int32_t chestAvailableGold = 0;
					
                    if (goldForm) {                     
                        if (merchantRef) {
                            auto actorInv = merchantRef->GetInventory();
                            auto it = actorInv.find(goldForm);
                            if (it != actorInv.end()) {
                                pocketChange += it->second.first; 
                            }
                        }
                        if (merchantChest) {
                            auto chestInv = merchantChest->GetInventory();
                            auto it = chestInv.find(goldForm);
                            if (it != chestInv.end()) {
                                chestAvailableGold += it->second.first; 
                            }
                        }
                    }
                    if (cfg->enableDebug) {
                        RE::DebugNotification(std::format("[DEBUG] Pocket Money: {} | Chest Gold: {} | Change Owed: {} ", pocketChange, chestAvailableGold, changeOwed).c_str());
                    }
                    int32_t totalChangeToDeduct = std::min(changeOwed, pocketChange + chestAvailableGold);

                    if (totalChangeToDeduct > 0) {
                        int32_t wastedGold = changeOwed - totalChangeToDeduct;
                        player->AddObjectToContainer(goldForm, nullptr, totalChangeToDeduct, nullptr);

                        RE::DebugNotification(TradeInGems::SafeFormat(cfg->msgChangeReceived, TradeInGems::Defaults::msgChangeReceived,
                            cfg->colorSystemHeader, cfg->colorGoldAsset, totalChangeToDeduct, cfg->labelGoldAbbr, cfg->colorSystemHeader, cfg->colorGoldAsset, wastedGold, cfg->labelGoldAbbr).c_str());

                        int32_t remainingToDeduct = totalChangeToDeduct;
                        int32_t deductFromChest = merchantChest ? std::min(remainingToDeduct, chestAvailableGold) : 0;
                        remainingToDeduct -= deductFromChest;
                        int32_t deductFromActor = std::min(remainingToDeduct, pocketChange);

                        if (deductFromChest > 0 && merchantChest) {
                            merchantChest->RemoveItem(goldForm, deductFromChest, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                        }
                        if (deductFromActor > 0 && merchantRef) {
                            merchantRef->RemoveItem(goldForm, deductFromActor, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                        }
                    }
                    else {
                        RE::DebugNotification(TradeInGems::SafeFormat(cfg->msgNoChange, TradeInGems::Defaults::msgNoChange,
                            cfg->colorSystemHeader, cfg->colorGoldAsset, changeOwed, cfg->labelGoldAbbr).c_str());
                    }
				} 
			} 
		} 
	} 

    // (Trading toggle): Controls Trading Mode On/Off for money injection and settling
    bool ToggleTradingMode() {
        auto cfg = g_settings.load();
        auto ui = RE::UI::GetSingleton();
        bool isInMenu = ui && (ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) || ui->IsMenuOpen(RE::BarterMenu::MENU_NAME));

        if (isInMenu) {
            RE::DebugNotification(TradeInGems::SafeFormat(cfg->msgTradingStatusMsg, TradeInGems::Defaults::msgTradingStatusMsg,
                cfg->colorSystemHeader, cfg->colorLootAsset, cfg->currencyName, cfg->colorSystemHeader).c_str());
            return false;
        }

        bool newModEnabledStatus = !g_modEnabled.load();
        g_modEnabled.store(newModEnabledStatus);

        std::string statusLabel = newModEnabledStatus ?
            std::format("<font color='#{}'>{}</font>", cfg->colorStatusActive, cfg->labelOn) :
            std::format("<font color='#{}'>{}</font>", cfg->colorStatusInactive, cfg->labelOff);

        RE::DebugNotification(TradeInGems::SafeFormat(cfg->msgTradingStatusToggle, TradeInGems::Defaults::msgTradingStatusToggle,
            cfg->colorLootAsset, cfg->currencyName, cfg->colorSystemHeader, statusLabel).c_str());

        return true;
    }

    // (Wealth display): Displays wealth and some extra optional lines on the current variables of interest
    void DisplayWealthInfo() {
        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        auto ui = RE::UI::GetSingleton();
        bool isInMenu = ui && (ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) || ui->IsMenuOpen(RE::BarterMenu::MENU_NAME));

        auto cfg = g_settings.load();
        TradeInGems::g_ecoCache.Refresh();

        auto avOwner = player->AsActorValueOwner();

        float rawSpeech = avOwner ? avOwner->GetActorValue(RE::ActorValue::kSpeech) : 0.0f;
        float flatMod = (avOwner && cfg->includePowerModifiers) ? avOwner->GetActorValue(RE::ActorValue::kSpeechcraftModifier) : 0.0f;
        float effectiveSpeech = std::clamp(rawSpeech + flatMod, 0.0f, 100.0f);

        RE::Actor* merchantActor = nullptr;
        if (auto smartPtr = g_capturedMerchantHandle.get()) {
            merchantActor = smartPtr->As<RE::Actor>();
        }

        int32_t gemGold = GetTotalGemWealthFast(merchantActor);
        auto goldForm = RE::TESForm::LookupByID<RE::TESBoundObject>(cfg->goldFormID);
        int32_t realGold = goldForm ? player->GetItemCount(goldForm) : 0;
        int32_t activeGhost = g_ghostGoldAdded.load();

        if (activeGhost > 0 && isInMenu) {
            realGold = (realGold >= activeGhost) ? (realGold - activeGhost) : 0;
        }

        bool isBuyMode = (cfg->itemValueMode == static_cast<int32_t>(ItemValueMode::kBuy));
        float merchantMultiplier = GetMerchantBarterMultiplier(isBuyMode, merchantActor);
        int32_t expectedPercent = static_cast<int32_t>(std::lround(isBuyMode ? std::max(100.0f * 1.05f, 100.0f * merchantMultiplier) : std::min(100.0f * merchantMultiplier, 100.0f)));

        std::string modeLabel = "";
        if (cfg->itemValueMode == static_cast<int32_t>(ItemValueMode::kBaseValue)) {
            modeLabel = cfg->labelModeFixed;
        }
        else if (cfg->itemValueMode == static_cast<int32_t>(ItemValueMode::kBuy)) {
            modeLabel = cfg->labelModeCurrency;
        }
        else {
            modeLabel = cfg->labelModeBarter;
        }

        std::string priorityColor = cfg->spendItemsBeforeGold ? cfg->colorLootAsset : cfg->colorGoldAsset;
        std::string priorityName = cfg->spendItemsBeforeGold ? cfg->currencyName : cfg->labelGold;

        // Framework info Line (optional)
        if (cfg->showModeStatus) {
            std::string walletStatus = g_modEnabled.load() ?
                std::format("<font color='#{}'>{}</font>", cfg->colorStatusActive, cfg->labelOn) :
                std::format("<font color='#{}'>{}</font>", cfg->colorStatusInactive, cfg->labelOff);

            RE::DebugNotification(TradeInGems::SafeFormat(cfg->msgFrameworkStatus, TradeInGems::Defaults::msgFrameworkStatus,
                cfg->colorSystemHeader, cfg->currencyName, walletStatus, cfg->colorSystemHeader, modeLabel,
                priorityColor, priorityName, cfg->colorSystemHeader, static_cast<int32_t>(effectiveSpeech)).c_str());
        }

        // Expected value percentage line (optional)
        if (cfg->showExpectedValue) {
            RE::DebugNotification(TradeInGems::SafeFormat(cfg->msgExpectedValue, TradeInGems::Defaults::msgExpectedValue,
                cfg->colorExpectedValue, cfg->currencyName, expectedPercent).c_str());
        }

        std::string assetLine = TradeInGems::SafeFormat(cfg->msgAssetSummary, TradeInGems::Defaults::msgAssetSummary,
            cfg->colorGoldAsset, cfg->labelGold, realGold, cfg->colorSystemHeader, cfg->colorLootAsset, cfg->currencyName, gemGold, cfg->colorSystemHeader, cfg->colorTotalAsset, (realGold + gemGold));

        RE::DebugNotification(assetLine.c_str());

        if (cfg->enableDebug && avOwner) {
            float percentPower = avOwner->GetActorValue(RE::ActorValue::kSpeechcraftPowerModifier);

            std::string sampleItemName = "None";
            int32_t sampleItemWorth = 0;
            float buyPerkPercent = 100.0f;
            float sellPerkPercent = 100.0f;

            if (!ItemValues.empty()) {
                auto it = ItemValues.begin();
                std::advance(it, rand() % ItemValues.size());

                if (auto sampleItem = RE::TESForm::LookupByID<RE::TESBoundObject>(it->first)) {
                    sampleItemName = sampleItem->GetName();
                    if (sampleItemName.empty()) sampleItemName = "Unnamed Item";
                    float rawVal = static_cast<float>(sampleItem->GetGoldValue());
                    sampleItemWorth = static_cast<int32_t>(std::lround(isBuyMode ? std::max(rawVal * 1.05f, rawVal * merchantMultiplier) : std::min(rawVal * merchantMultiplier, rawVal)));
                }
            }

            if (cfg->includePerkModifiers) {
                buyPerkPercent = ApplyPerkPriceModifier(100.0f, true, merchantActor);
                sellPerkPercent = ApplyPerkPriceModifier(100.0f, false, merchantActor);
            }

            std::string debugMsg = std::format(
                "[DEBUG] kSpeech: {} | kSpeechcraftModifier: {} | Boosts: {}% | Perks on: {} | Sample [{}]: {}g | B Perks: {:.1f}% | S Perks: {:.1f}%",
                rawSpeech, flatMod, percentPower, cfg->includePerkModifiers, sampleItemName, sampleItemWorth, buyPerkPercent, sellPerkPercent
            );

            if (auto console = RE::ConsoleLog::GetSingleton()) {
                console->Print(debugMsg.c_str());
            }
        }
    }

    // debug tool to check available commands to sacrifice
    void DebugScanSacrificialCommands(bool firstrun)
    {
        static const std::vector<const char*> kSacrificialCommands = {
            "StartAllQuests",
            "CompleteAllQuestStages",
            "ToggleFogOfWar",
            "ToggleWireframe",
            "ToggleWaterSystem",
            "ToggleSky",
            "ToggleTrees",
            "CenterOnWorld"
        };

        // Use the main default logger instead of creating a custom basic_file_sink
        if (auto scanLogger = spdlog::default_logger()) {
            scanLogger->info("=== TradeInGems LACE Console Command Scan Log ({}) ===", firstrun ? "Pre-Hook" : "Post-Hook");

            for (const char* sacrificialName : kSacrificialCommands) {
                if (auto cmd = RE::SCRIPT_FUNCTION::LocateConsoleCommand(sacrificialName)) {
                    scanLogger->info("[FOUND]     {:<25} Address: 0x{:X}",sacrificialName,reinterpret_cast<std::uintptr_t>(cmd));
                }
                else {
                    scanLogger->info("[NOT FOUND] {}", sacrificialName);
                }
            }
        }
    }

    namespace ConsoleCommands
    {
        void PrintConsole(const std::string& a_msg) {
            if (auto console = RE::ConsoleLog::GetSingleton()) {
                console->Print("%s", a_msg.c_str());
            }
        }

        void SaveUserLootToPatchFile(const std::string& a_editorID) {
            std::filesystem::path patchPath = "Data/SKSE/Plugins/TradeInGemsLACE/Patches/ConsoleAdded.txt";
            std::filesystem::create_directories(patchPath.parent_path());

            std::ofstream file(patchPath, std::ios::app);
            if (file.is_open()) {
                file << a_editorID << "\n";
            }
        }

        bool ExecuteTGLace(
            const RE::SCRIPT_PARAMETER* /*a_paramInfo*/,
            RE::SCRIPT_FUNCTION::ScriptData* a_scriptData,
            RE::TESObjectREFR* a_thisObj,
            RE::TESObjectREFR* /*a_containingObj*/,
            RE::Script* /*a_scriptObj*/,
            RE::ScriptLocals* /*a_locals*/,
            double& a_result,
            std::uint32_t& /*a_opcodeOffsetPtr*/)
        {
            a_result = 0.0;
            std::string input;

            if (a_scriptData && a_scriptData->numParams > 0) {
                if (auto strChunk = a_scriptData->GetStringChunk()) {
                    input = strChunk->GetString();
                }
            }

            std::istringstream iss(input);
            std::string action;
            iss >> action;

            std::transform(action.begin(), action.end(), action.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (action == "ini") {
                if (g_ghostGoldAdded.load() > 0) {
                    PrintConsole("TradeInGems LACE: Cannot reload INI settings while injected gold is active. Settle injected gold first (exit dialogue or type 'lace settle').");
                    return true;
                }
                TradeInGems::LoadSettings();
                PrintConsole("TradeInGems LACE: Settings reloaded from INI file.");
                a_result = 1.0;
            }
            else if (action == "rescan") {
                TradeInGems::ItemValues.clear();
                TradeInGems::AppendModdedGems(TradeInGems::ItemValues);
                PrintConsole(std::format("TradeInGems LACE: Item registry reloaded and written to log. Total tracked items: {}", TradeInGems::ItemValues.size()));
                a_result = 1.0;
            }
            else if (action == "add") {
                std::string editorID;
                iss >> editorID;

                RE::TESBoundObject* targetLoot = nullptr;

                if (!editorID.empty()) {
                    targetLoot = RE::TESForm::LookupByEditorID<RE::TESBoundObject>(editorID);
                }
                if (!targetLoot && a_thisObj) {
                    targetLoot = a_thisObj->GetBaseObject();
                }

                if (!targetLoot) {
                    PrintConsole("TradeInGems LACE: Usage -> lace add <EditorID> (or select an item in console mode and type 'lace add')");
                    return true;
                }
                if (!targetLoot->IsInventoryObject()) {
                    PrintConsole("TradeInGems LACE: Target is not a valid tradeable inventory item.");
                    return true;
                }
                if (editorID.empty()) {
                    editorID = GetEditorIDFromFormID(targetLoot->GetFormID());
                }

                RE::FormID formID = targetLoot->GetFormID();
                std::string itemName = targetLoot->GetName();
                if (itemName.empty()) itemName = "Unknown Item";

                TradeInGems::ItemValues[formID] = 0;

                if (editorID.empty()) {
                    PrintConsole(std::format("TradeInGems LACE: Failed to retrieve EditorID for '{}' (0x{:08X}). Ensure po3_Tweaks is installed.", itemName, formID));
                    return true;
                }

                SaveUserLootToPatchFile(editorID);
                PrintConsole(std::format("TradeInGems LACE: Added '{}' ({}) to custom patch list.", itemName, editorID));
                a_result = 1.0;
            }
            else if (action == "toggle") {
                if (ToggleTradingMode()) {
                    PrintConsole(std::format("TradeInGems LACE: Trading mode is now {}.", g_modEnabled.load() ? "ENABLED" : "DISABLED"));
                }
                else {
                    PrintConsole("TradeInGems LACE: Cannot toggle trading mode while in Dialogue/Barter Menu.");
                }
                a_result = 1.0;
            }
            else if (action == "settle") {
                if (g_ghostGoldAdded.load() <= 0) {
                    PrintConsole("TradeInGems LACE: No injected gold to settle.");
                    return true;
                }
                auto ui = RE::UI::GetSingleton();
                if (ui) {
                    bool isInMenu = ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) ||
                        ui->IsMenuOpen(RE::BarterMenu::MENU_NAME) ||
                        ui->IsMenuOpen(RE::ContainerMenu::MENU_NAME) ||
                        ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME) ||
                        ui->IsMenuOpen(RE::TweenMenu::MENU_NAME);

                    if (isInMenu) {
                        PrintConsole("TradeInGems LACE: Cannot settle while a menu is open. Close menus first.");
                        return true;
                    }
                }

                if (!a_thisObj || !a_thisObj->As<RE::Actor>()) {
                    PrintConsole("TradeInGems LACE: Settlement failed. You must click on a valid NPC/Actor while in the console.");
                    return true;
                }

                Settle(a_thisObj);

                std::string actorName = a_thisObj->GetDisplayFullName();
                if (actorName.empty()) actorName = "Unnamed Actor";

                PrintConsole(std::format("TradeInGems LACE: Settled injected gold for target '{}' (0x{:X}).", actorName, a_thisObj->GetFormID()));
                a_result = 1.0;
            }
            else if (action == "wealth") {
                DisplayWealthInfo();
                PrintConsole("TradeInGems LACE: Wealth info displayed on screen.");
                a_result = 1.0;
            }
            else if (action.empty()) {
                PrintConsole("TradeInGems LACE: [WARNING] No subcommand specified.");
                PrintConsole("Usage: [tig-lace | lace] [add | ini | rescan | toggle | settle | wealth]");
            }
            else {
                PrintConsole(std::format("TradeInGems LACE: [WARNING] Unknown subcommand '{}'.", action));
                PrintConsole("Valid subcommands: add <ID>, ini, rescan, toggle, settle, wealth");
            }

            return true;
        }

        void Register() {
            static RE::SCRIPT_PARAMETER kCommandParam[] = {
                { "Arguments", RE::SCRIPT_PARAM_TYPE::kChar, 1 } // 1 = OPTIONAL parameter
            };

            static constexpr std::array kSacrificialCommands = {
                "StartAllQuests",
                "CompleteAllQuestStages",
                "ToggleFogOfWar",
                "ToggleWireframe",
                "ToggleWaterSystem",
                "ToggleSky",
                "ToggleTrees",
                "CenterOnWorld"
            };

            for (const char* sacrificialName : kSacrificialCommands) {
                if (auto cmd = RE::SCRIPT_FUNCTION::LocateConsoleCommand(sacrificialName)) {
                    cmd->functionName = "tig-lace";
                    cmd->shortName = "lace";
                    cmd->helpString = "TradeInGems LACE Master Command. Usage: [tig-lace | lace] [add | ini | rescan | toggle | settle | wealth]";
                    cmd->referenceFunction = false;

                    cmd->executeFunction = &ExecuteTGLace;
                    cmd->numParams = 1;
                    cmd->params = kCommandParam;
                    return;
                }
            }
        }
    }

	// younger brother of the two event handlers, this one is faster but less reliable than MenuWatcher, it will trigger when the player presses the interaction button
    class ActivateHandler : public RE::BSTEventSink<RE::TESActivateEvent> {
    public:
        static ActivateHandler* GetSingleton() { static ActivateHandler singleton; return &singleton; }

        RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* a_event, RE::BSTEventSource<RE::TESActivateEvent>* a_eventSource) override {
            if (!a_event) return RE::BSEventNotifyControl::kContinue;

            auto cfg = g_settings.load();
            bool modEnabled = g_modEnabled.load();

            if (!cfg->useActivateHandler || (!modEnabled && !cfg->cursedGoldMode)) {
                return RE::BSEventNotifyControl::kContinue;
            }

            auto actionRef = a_event->actionRef.get();
            auto targetRef = a_event->objectActivated.get();

            if (actionRef && actionRef->IsPlayerRef() && targetRef) {
                if (auto actor = targetRef->As<RE::Actor>()) {
                    g_capturedMerchantHandle = targetRef->CreateRefHandle();
                    Inject(targetRef);

                    RE::ObjectRefHandle merchantHandle = g_capturedMerchantHandle;
                    SKSE::GetTaskInterface()->AddTask([merchantHandle]() {
                        if (g_ghostGoldAdded.load() <= 0) return;

                        auto ui = RE::UI::GetSingleton();
                        bool menuIsOpen = ui && (ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) ||
                            ui->IsMenuOpen(RE::BarterMenu::MENU_NAME) ||
                            ui->IsMenuOpen(RE::ContainerMenu::MENU_NAME));
                        if (!menuIsOpen) {
                            if (g_capturedMerchantHandle == merchantHandle) {
                                g_capturedMerchantHandle.reset();
                            }

                            auto merchantPtr = merchantHandle.get();
                            Settle(merchantPtr ? merchantPtr.get() : nullptr);
                        }
                        });
                    if (cfg->enableDebug) {
                        std::string msg = std::format("[DEBUG] ActivateHandler's EarlyGoldInjection done for: {} (0x{:X})",
                            targetRef->GetDisplayFullName(), targetRef->GetFormID());
                        RE::DebugNotification(msg.c_str());
                        if (auto console = RE::ConsoleLog::GetSingleton()) console->Print(msg.c_str());
                    }
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };

	// older brother of the two event handlers, it is slower but more reliable than ActivateHandler, it will trigger when the dialogue menu opens and closes (double safety net)
    // also has safety and fallback logic for exceptional cases where the dialogue menu methods aren't enough 
    class MenuWatcher : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        static MenuWatcher* GetSingleton() { static MenuWatcher singleton; return &singleton; }
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
            auto cfg = g_settings.load();
            bool modEnabled = g_modEnabled.load();
            bool ghostActive = g_ghostGoldAdded.load() > 0;
            if (!a_event || (!modEnabled && !cfg->cursedGoldMode && !ghostActive)) {
                return RE::BSEventNotifyControl::kContinue;
            }
			// Dialogue menu opens -> inject, Dialogue menu closes -> settle (expected normal behavior)
            if (a_event->menuName == RE::DialogueMenu::MENU_NAME) {
                if (a_event->opening) {
                    if (!cfg->useActivateHandler || !g_capturedMerchantHandle) {
                        if (auto topicManager = RE::MenuTopicManager::GetSingleton()) {
                            g_capturedMerchantHandle = topicManager->speaker ? topicManager->speaker : topicManager->lastSpeaker;
                        }
                    }
                    auto merchantRef = g_capturedMerchantHandle.get();
                    Inject(merchantRef ? merchantRef.get() : nullptr);
                }
                else {
                    RE::ObjectRefHandle merchantHandle = g_capturedMerchantHandle;
                    g_capturedMerchantHandle.reset();

                    SKSE::GetTaskInterface()->AddTask([merchantHandle]() {
                        auto merchantPtr = merchantHandle.get();
                        Settle(merchantPtr ? merchantPtr.get() : nullptr);
                        });
                }
            }
			// Barter menu opens without dialogue menu opening first (unexpected but supported) -> injects so you get gold for the barter session
            else if (a_event->menuName == RE::BarterMenu::MENU_NAME && a_event->opening) {
                if (g_ghostGoldAdded.load() == 0) {
                    RE::TESObjectREFR* targetMerchant = nullptr;

                    if (auto smartPtr = g_capturedMerchantHandle.get()) {
                        targetMerchant = smartPtr.get();
                    }
                    if (!targetMerchant) {
                        if (auto topicManager = RE::MenuTopicManager::GetSingleton()) {
                            auto handle = topicManager->speaker ? topicManager->speaker : topicManager->lastSpeaker;
                            if (auto smartPtr = handle.get()) {
                                targetMerchant = smartPtr.get();
                            }
                        }
                    }
                    if (targetMerchant && targetMerchant->As<RE::Actor>()) {
                        g_capturedMerchantHandle = targetMerchant->CreateRefHandle();
                        Inject(targetMerchant);
                    }
                }
            }
            // Safety settle for any remaining ghost gold when returning to the main game view, will settle the gold injected from no dialogue Barter menu entry as well
            else if (a_event->menuName == RE::HUDMenu::MENU_NAME && a_event->opening) {
                if (g_ghostGoldAdded.load() > 0) {
                    // store the global handle for the final settle attempt, then drop it (the next gold injection will create the next one)
                    RE::ObjectRefHandle merchantHandle = g_capturedMerchantHandle;
                    g_capturedMerchantHandle.reset();

                    SKSE::GetTaskInterface()->AddTask([merchantHandle]() {
                        auto merchantPtr = merchantHandle.get();
                        Settle(merchantPtr ? merchantPtr.get() : nullptr);
                        });
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };

    class InputHandler : public RE::BSTEventSink<RE::InputEvent*> {
    public:
        static InputHandler* GetSingleton() { static InputHandler singleton; return &singleton; }

        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override {
            if (!a_event) return RE::BSEventNotifyControl::kContinue;
            auto cfg = g_settings.load();

            for (auto event = *a_event; event; event = event->next) {
                auto button = event->AsButtonEvent();
                if (button && button->IsDown()) {
                    uint32_t key = button->GetIDCode();

                    if (key == cfg->toggleKey) {ToggleTradingMode();}
                    else if (key == cfg->wealthKey) {DisplayWealthInfo();}
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    TradeInGems::InitializeFirstLoadSettings();
    TradeInGems::LoadSettings();
    TradeInGems::LoadMenuTexts();
    SetupLog();

    SKSE::GetMessagingInterface()->RegisterListener("SKSE", [](SKSE::MessagingInterface::Message* msg) {
        if (msg->type == SKSE::MessagingInterface::kDataLoaded) {
            TradeInGems::DebugScanSacrificialCommands(true);
            TradeInGems::ConsoleCommands::Register();
            TradeInGems::DebugScanSacrificialCommands(false);
            TradeInGems::MCPLaceMenu::Register();
            TradeInGems::g_ecoCache.Refresh();
            TradeInGems::AppendModdedGems(TradeInGems::ItemValues);

            RE::UI::GetSingleton()->AddEventSink(TradeInGems::MenuWatcher::GetSingleton());
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(TradeInGems::ActivateHandler::GetSingleton()); 

        }
        else if (msg->type == SKSE::MessagingInterface::kInputLoaded) {
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink(TradeInGems::InputHandler::GetSingleton());
        }
        else if (msg->type == SKSE::MessagingInterface::kPostLoadGame ||
            msg->type == SKSE::MessagingInterface::kPreLoadGame)
        {
            TradeInGems::g_ghostGoldAdded.store(0);
            TradeInGems::g_realGoldBeforeTransaction.store(0);
            TradeInGems::LoadSettings();
            TradeInGems::SetGemsHidden(false);
        }
        else if (msg->type == SKSE::MessagingInterface::kNewGame)
        {
            TradeInGems::g_ghostGoldAdded.store(0);
            TradeInGems::g_realGoldBeforeTransaction.store(0);
            TradeInGems::InitializeFirstLoadSettings();
            TradeInGems::LoadSettings();
            TradeInGems::SetGemsHidden(false);
        }
        });

    return true;
}

