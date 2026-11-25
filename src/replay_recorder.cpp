#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <chrono>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace {
    struct RecordedInput {
        std::uint64_t microseconds;
        int button;
        bool down;
    };

    std::vector<RecordedInput> s_recordedInputs;
    std::chrono::steady_clock::time_point s_recordingStart;
    bool s_hasRecordingStart = false;

    void recordInput(PlayerButton button, bool down) {
        if (!Mod::get()->getSettingValue<bool>("record-inputs")) {
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        if (!s_hasRecordingStart) {
            s_recordingStart = now;
            s_hasRecordingStart = true;
        }

        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            now - s_recordingStart
        ).count();

        s_recordedInputs.push_back({
            static_cast<std::uint64_t>(elapsed),
            static_cast<int>(button),
            down
        });
    }

    void saveRecording() {
        if (s_recordedInputs.empty()) {
            return;
        }

        std::ostringstream output;
        output << "GDRT-REPLAY,1\n";
        output << "time_us,button,down\n";

        for (const auto& input : s_recordedInputs) {
            output << input.microseconds << ','
                   << input.button << ','
                   << (input.down ? 1 : 0) << '\n';
        }

        Mod::get()->setSavedValue<std::string>("last-replay-csv", output.str());
        log::info("Saved {} replay input events to Geode saved data.", s_recordedInputs.size());
    }
}

class $modify(GDRTPlayerObject, PlayerObject) {
    void pushButton(PlayerButton button) {
        recordInput(button, true);
        PlayerObject::pushButton(button);
    }

    void releaseButton(PlayerButton button) {
        recordInput(button, false);
        PlayerObject::releaseButton(button);
    }
};

class $modify(GDRTPlayLayer, PlayLayer) {
    void onQuit() {
        saveRecording();
        PlayLayer::onQuit();
    }
};

$on_mod(Loaded) {
    log::info("GD Replay Tools input recorder loaded.");
    auto saved = Mod::get()->getSavedValue<std::string>("last-replay-csv", "");
    if (!saved.empty()) {
        log::info("A saved replay is available ({} bytes).", saved.size());
    }
}
