#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include <chrono>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace gdrt {
    struct RecordedInput {
        double gameSeconds = 0.0;
        int button = 0;
        bool down = false;
    };

    static std::vector<RecordedInput> s_events;
    static std::vector<RecordedInput> s_playback;
    static std::chrono::steady_clock::time_point s_recordStart;
    static double s_playTime = 0.0;
    static std::size_t s_playIndex = 0;
    static bool s_recording = false;
    static bool s_playing = false;

    static float speedMultiplier() {
        if (!Mod::get()->getSettingValue<bool>("speedhack-enabled")) return 1.f;
        return Mod::get()->getSettingValue<float>("speedhack-multiplier");
    }

    static std::string serialize(std::vector<RecordedInput> const& events) {
        std::ostringstream out;
        out << "GDRT-REPLAY,2\n";
        out.precision(9);
        for (auto const& event : events) {
            out << event.gameSeconds << ',' << event.button << ',' << (event.down ? 1 : 0) << '\n';
        }
        return out.str();
    }

    static std::vector<RecordedInput> parse(std::string const& data) {
        std::vector<RecordedInput> result;
        std::istringstream input(data);
        std::string line;
        if (!std::getline(input, line) || line.rfind("GDRT-REPLAY,", 0) != 0) return result;
        while (std::getline(input, line)) {
            std::istringstream row(line);
            std::string time, button, down;
            if (!std::getline(row, time, ',') || !std::getline(row, button, ',') || !std::getline(row, down, ',')) continue;
            try {
                result.push_back({std::stod(time), std::stoi(button), std::stoi(down) != 0});
            } catch (...) {
                continue;
            }
        }
        return result;
    }

    void startRecording() {
        s_events.clear();
        s_recordStart = std::chrono::steady_clock::now();
        s_recording = true;
        s_playing = false;
        s_playback.clear();
        s_playIndex = 0;
        Mod::get()->setSettingValue("record-inputs", true);
        log::info("Replay recording started.");
    }

    void stopRecording() {
        s_recording = false;
        Mod::get()->setSettingValue("record-inputs", false);
        log::info("Replay recording stopped ({} events).", s_events.size());
    }

    bool isRecording() { return s_recording; }
    bool hasSavedReplay() {
        return !Mod::get()->getSavedValue<std::string>("last-replay-csv", "").empty();
    }

    void saveRecording() {
        if (s_recording) stopRecording();
        if (s_events.empty()) return;
        auto data = serialize(s_events);
        Mod::get()->setSavedValue<std::string>("last-replay-csv", data);
        log::info("Replay saved ({} events).", s_events.size());
    }

    int getTemplateCount() {
        return Mod::get()->getSavedValue<int>("replay-template-count", 0);
    }

    void createTemplate() {
        auto data = Mod::get()->getSavedValue<std::string>("last-replay-csv", "");
        if (data.empty()) return;
        int count = getTemplateCount();
        Mod::get()->setSavedValue<std::string>("replay-template-" + std::to_string(count + 1), data);
        Mod::get()->setSavedValue<int>("replay-template-count", count + 1);
        log::info("Created replay template #{}.", count + 1);
    }

    void deleteTemplate() {
        int count = getTemplateCount();
        if (count <= 0) return;
        Mod::get()->setSavedValue<std::string>("replay-template-" + std::to_string(count), "");
        Mod::get()->setSavedValue<int>("replay-template-count", count - 1);
        log::info("Deleted the latest replay template.");
    }

    void startPlayback() {
        auto data = Mod::get()->getSavedValue<std::string>("last-replay-csv", "");
        if (data.empty()) {
            int count = getTemplateCount();
            if (count > 0) data = Mod::get()->getSavedValue<std::string>("replay-template-" + std::to_string(count), "");
        }
        s_playback = parse(data);
        s_playIndex = 0;
        s_playTime = 0.0;
        s_playing = !s_playback.empty();
        s_recording = false;
        Mod::get()->setSettingValue("record-inputs", false);
        if (s_playing) log::info("Replay playback started.");
        else log::info("No saved replay to play.");
    }

    void stopPlayback() {
        s_playing = false;
        s_playback.clear();
        s_playIndex = 0;
    }

    void advancePlayback(PlayerObject* player, float gameDelta) {
        if (!s_playing || !player || gameDelta <= 0.f) return;
        s_playTime += gameDelta;
        while (s_playIndex < s_playback.size() && s_playback[s_playIndex].gameSeconds <= s_playTime) {
            auto const& event = s_playback[s_playIndex++];
            auto button = static_cast<PlayerButton>(event.button);
            if (event.down) player->pushButton(button);
            else player->releaseButton(button);
        }
        if (s_playIndex >= s_playback.size()) stopPlayback();
    }

    static void recordInput(PlayerButton button, bool down) {
        if (!s_recording || s_playing) return;
        auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - s_recordStart).count();
        s_events.push_back({elapsed * speedMultiplier(), static_cast<int>(button), down});
    }
}

class $modify(GDRTPlayerObject, PlayerObject) {
    void pushButton(PlayerButton button) {
        gdrt::recordInput(button, true);
        PlayerObject::pushButton(button);
    }

    void releaseButton(PlayerButton button) {
        gdrt::recordInput(button, false);
        PlayerObject::releaseButton(button);
    }
}
