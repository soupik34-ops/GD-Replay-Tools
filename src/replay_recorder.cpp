#include <Geode/Geode.hpp>

#include <chrono>
#include <cstdint>
#include <vector>

using namespace geode::prelude;

namespace {
    struct RecordedInput {
        std::uint64_t milliseconds;
        int button;
    };

    std::vector<RecordedInput> s_recordedInputs;
    std::chrono::steady_clock::time_point s_recordingStart;
}

class $modify(GDRTPlayerObject, PlayerObject) {
    void pushButton(PlayerButton button) {
        if (Mod::get()->getSettingValue<bool>("record-inputs")) {
            if (s_recordedInputs.empty()) {
                s_recordingStart = std::chrono::steady_clock::now();
            }

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - s_recordingStart
            ).count();

            s_recordedInputs.push_back({
                static_cast<std::uint64_t>(elapsed),
                static_cast<int>(button)
            });
        }

        PlayerObject::pushButton(button);
    }
};

$on_mod(Loaded) {
    log::info("GD Replay Tools input recorder prototype loaded.");
}
