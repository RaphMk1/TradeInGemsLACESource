#include "GemRegistry.h"
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace TradeInGems {
    void AppendModdedGems(std::unordered_map<RE::FormID, int32_t>& a_gemMap) {
        SKSE::log::info("=== TradeInGems LACE Item Registry Scan Log ===");
        std::filesystem::path patchFolder = "Data/SKSE/Plugins/TradeInGemsLACE/Patches";

        if (!std::filesystem::exists(patchFolder)) {
            SKSE::log::info("TradeInGems LACE Log -> No patches folder found.");
            return;
        }

        std::vector<std::string> moddedEditorIDs;

        for (const auto& entry : std::filesystem::directory_iterator(patchFolder)) {
            if (entry.path().extension() == ".txt") {
                std::ifstream file(entry.path());
                std::string line;

                while (std::getline(file, line)) {
                    auto commentPos = line.find_first_of(";#");
                    if (commentPos != std::string::npos) {
                        line = line.substr(0, commentPos);
                    }
                    auto slashCommentPos = line.find("//");
                    if (slashCommentPos != std::string::npos) {
                        line = line.substr(0, slashCommentPos);
                    }
                    line.erase(std::remove_if(line.begin(), line.end(), [](unsigned char c) {
                        return !(std::isalnum(c) || c == '_' || c == '-');
                        }), line.end());

                    if (!line.empty()) {
                        moddedEditorIDs.push_back(line);
                    }
                }
            }
        }

        SKSE::log::info("TradeInGems LACE Log -> Total EditorID tokens parsed from text files: {}", moddedEditorIDs.size());

        for (const auto& editorID : moddedEditorIDs) {
            auto gem = RE::TESForm::LookupByEditorID<RE::TESBoundObject>(editorID);
            if (gem) {
                a_gemMap[gem->GetFormID()] = 0;

                SKSE::log::info("  [SUCCESS] Registered: {} | FormID: 0x{:08X}", editorID, gem->GetFormID());
            }
            else {
                SKSE::log::info("  [FAILED] No editorID match found for string: \"{}\"", editorID);
            }
        }

        SKSE::log::info("TradeInGems LACE Log -> Processing complete. Total stored map capacity: {}", a_gemMap.size());
    }
}
