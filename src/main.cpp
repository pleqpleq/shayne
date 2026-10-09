#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <fmod.hpp>

#include "TapSpeed.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <random>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace {
    enum class Category : size_t {
        NormalPress, NormalRelease, FastPress, FastRelease, SpacePress, SpaceRelease, Count
    };
    constexpr size_t kCategoryCount = static_cast<size_t>(Category::Count);

    struct CategoryInfo {
        const char* prefix;
        int count;
    };
    constexpr std::array<CategoryInfo, kCategoryCount> kInfo = {{
        {"normal_clicks_", 16}, {"normal_releases_", 16},
        {"fast_clicks_", 24}, {"fast_releases_", 23},
        {"space_clicks_", 15}, {"space_releases_", 15}
    }};

    class SoundBank {
    public:
        void preload() {
            if (m_loaded) return;
            auto* engine = FMODAudioEngine::get();
            if (!engine || !engine->m_system) return;
            auto* system = engine->m_system;
            auto base = Mod::get()->getResourcesDir();

            for (size_t i = 0; i < kCategoryCount; ++i) {
                // Geode extracts resources/*.wav into the mod resource directory.
                for (int j = 1; j <= kInfo[i].count; ++j) {
                    std::string name = kInfo[i].prefix;
                    if (j < 10) name += '0';
                    name += std::to_string(j) + ".wav";
                    auto path = base / name;
                    FMOD::Sound* sound = nullptr;
                    const auto status = system->createSound(
                        path.string().c_str(), FMOD_DEFAULT, nullptr, &sound
                    );
                    if (status == FMOD_OK && sound) {
                        m_samples[i].push_back(sound);
                    } else {
                        log::warn("Could not load click sample: {} (FMOD {})", name, static_cast<int>(status));
                    }
                }
            }
            m_loaded = true;
            log::info("Supleq Smart Clicks: loaded {} WAV samples", sampleCount());
        }

        void play(Category type, float volume) {
            if (volume <= 0.f) return;
            preload();
            auto& sounds = m_samples[static_cast<size_t>(type)];
            if (sounds.empty()) return;
            auto* engine = FMODAudioEngine::get();
            if (!engine || !engine->m_system) return;

            const size_t category = static_cast<size_t>(type);
            size_t index = 0;
            if (sounds.size() > 1) {
                std::uniform_int_distribution<size_t> dist(0, sounds.size()-2);
                index = dist(m_rng);
                // Uniform random draw excluding the immediately previous sample.
                if (index >= m_lastSample[category]) ++index;
            }
            m_lastSample[category] = index;

            // Start paused to ensure the volume is set before the sound begins.
            FMOD::Channel* channel = nullptr;
            const auto status = engine->m_system->playSound(sounds[index], nullptr, true, &channel);
            if (status == FMOD_OK && channel) {
                channel->setVolume(std::clamp(volume, 0.f, 1.f));
                channel->setPaused(false);
            }
        }

    private:
        size_t sampleCount() const {
            size_t n = 0;
            for (auto const& group : m_samples) n += group.size();
            return n;
        }
        bool m_loaded = false;
        std::array<std::vector<FMOD::Sound*>, kCategoryCount> m_samples;
        std::array<size_t, kCategoryCount> m_lastSample = [] {
            std::array<size_t, kCategoryCount> last{};
            last.fill(std::numeric_limits<size_t>::max());
            return last;
        }();
        std::mt19937 m_rng{std::random_device{}()};
        // FMOD owns allocated sound memory until its audio system shuts down.
    };

    SoundBank& bank() {
        static SoundBank instance;
        return instance;
    }

    struct InputState {
        bool upHeld = false;
        bool spaceHeld = false;
        bool upPressWasFast = false;
        TapSpeed upSpeed;
        void reset() {
            upHeld = false;
            spaceHeld = false;
            upPressWasFast = false;
            upSpeed.reset();
        }
    };

    InputState& state() {
        static InputState instance;
        return instance;
    }

    double secondsNow() {
        using Clock = std::chrono::steady_clock;
        return std::chrono::duration<double>(Clock::now().time_since_epoch()).count();
    }

    float upVolume() {
        return static_cast<float>(Mod::get()->getSettingValue<int64_t>("up-volume")) / 100.f;
    }
    float spaceVolume() {
        return static_cast<float>(Mod::get()->getSettingValue<int64_t>("space-volume")) / 100.f;
    }
    float releaseVolume() {
        return static_cast<float>(Mod::get()->getSettingValue<int64_t>("release-volume")) / 100.f;
    }

    void processKey(enumKeyCodes key, bool down, bool isRepeat) {
        if (key != KEY_Up && key != KEY_Space) return;

        auto* layer = PlayLayer::get();
        if (!layer || layer->m_isPaused || layer->m_hasCompletedLevel ||
            !Mod::get()->getSettingValue<bool>("enabled")) {
            // Never leave held inputs stuck when the game is paused or a level exits.
            state().reset();
            return;
        }

        auto& s = state();
        if (key == KEY_Up) {
            if (down) {
                if (isRepeat || s.upHeld) return;
                s.upHeld = true;
                bool fast = s.upSpeed.press(
                    secondsNow(), Mod::get()->getSettingValue<double>("fast-threshold")
                );
                s.upPressWasFast = fast && Mod::get()->getSettingValue<bool>("fast-enabled");
                bank().play(s.upPressWasFast ? Category::FastPress : Category::NormalPress, upVolume());
            } else {
                if (!s.upHeld) return;
                s.upHeld = false;
                bank().play(s.upPressWasFast ? Category::FastRelease : Category::NormalRelease,
                            upVolume() * releaseVolume());
            }
        } else {
            // Dedicated spacebar recording, independent of Up Arrow speed.
            if (down) {
                if (isRepeat || s.spaceHeld) return;
                s.spaceHeld = true;
                bank().play(Category::SpacePress, spaceVolume());
            } else {
                if (!s.spaceHeld) return;
                s.spaceHeld = false;
                bank().play(Category::SpaceRelease, spaceVolume() * releaseVolume());
            }
        }
    }
}

class $modify(SupleqKeyboardSounds, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool isRepeat, double timestamp) {
        // Preserve GD and every other mod's gameplay input handling.
        bool result = CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, timestamp);
        processKey(key, down, isRepeat);
        return result;
    }
};

class $modify(SupleqPlayLayerPreload, PlayLayer) {
    void onEnterTransitionDidFinish() override {
        PlayLayer::onEnterTransitionDidFinish();
        state().reset();
        bank().preload();
    }

    void onExit() override {
        state().reset();
        PlayLayer::onExit();
    }
};
