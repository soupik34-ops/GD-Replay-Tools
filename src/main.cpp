#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <functional>
#include <string>

using namespace geode::prelude;

namespace gdrt {
    void startRecording();
    void stopRecording();
    bool isRecording();
    void saveRecording();
    bool hasSavedReplay();
    void startPlayback();
    void stopPlayback();
    void advancePlayback(PlayerObject* player, float gameDelta);
    void createTemplate();
    void deleteTemplate();
    int getTemplateCount();
}

namespace {
    bool s_noclipEnabled = false;
    bool s_speedEnabled = false;

    class GDRTOverlay final : public CCLayer {
        CCLayer* m_panel = nullptr;
        CCLayer* m_toolsPage = nullptr;
        CCLayer* m_recordPage = nullptr;
        CCLabelBMFont* m_noclipStatus = nullptr;
        CCLabelBMFont* m_speedStatus = nullptr;
        CCLabelBMFont* m_recordStatus = nullptr;
        CCLabelBMFont* m_templateStatus = nullptr;
        float m_panelScale = 1.f;

        CCMenuItemSpriteExtra* makeButton(char const* text, char const* frame, std::function<void()> callback) {
            auto sprite = ButtonSprite::create(text, 132, 0, 0.62f, false, "goldFont.fnt", frame, 30.f);
            return CCMenuItemExt::createSpriteExtra(sprite, [callback](CCMenuItemSpriteExtra*) { callback(); });
        }

        void refreshLabels() {
            if (m_noclipStatus) {
                m_noclipStatus->setString(s_noclipEnabled ? "Geometry Dash: ON" : "Geometry Dash: OFF");
            }
            if (m_speedStatus) {
                float multiplier = Mod::get()->getSettingValue<float>("speedhack-multiplier");
                char text[64];
                std::snprintf(text, sizeof(text), "Speedhack: x%.2f %s", multiplier, s_speedEnabled ? "ON" : "OFF");
                m_speedStatus->setString(text);
            }
            if (m_recordStatus) {
                m_recordStatus->setString(gdrt::isRecording() ? "RECORDING..." : "RECORD: OFF");
            }
            if (m_templateStatus) {
                char text[64];
                std::snprintf(text, sizeof(text), "Templates: %d", gdrt::getTemplateCount());
                m_templateStatus->setString(text);
            }
        }

        void showTools() {
            m_toolsPage->setVisible(true);
            m_recordPage->setVisible(false);
            refreshLabels();
        }

        void showRecord() {
            m_toolsPage->setVisible(false);
            m_recordPage->setVisible(true);
            refreshLabels();
        }

        void toggleNoclip() {
            s_noclipEnabled = !s_noclipEnabled;
            Mod::get()->setSettingValue("noclip-enabled", s_noclipEnabled);
            refreshLabels();
        }

        void toggleSpeed() {
            s_speedEnabled = !s_speedEnabled;
            Mod::get()->setSettingValue("speedhack-enabled", s_speedEnabled);
            refreshLabels();
        }

        void changeSpeed(float amount) {
            float value = Mod::get()->getSettingValue<float>("speedhack-multiplier");
            value = std::clamp(value + amount, 0.25f, 5.0f);
            value = std::round(value * 20.f) / 20.f;
            Mod::get()->setSettingValue("speedhack-multiplier", value);
            refreshLabels();
        }

        bool init() {
            if (!CCLayer::init()) return false;
            auto win = CCDirector::get()->getWinSize();

            auto launcherMenu = CCMenu::create();
            launcherMenu->setPosition(CCPointZero);

            auto emblem = CCSprite::create("Emblem.png"_spr);
            if (!emblem) emblem = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
            if (emblem) {
                emblem->setScale(0.19f);
                auto launcher = CCMenuItemSpriteExtra::create(
                    emblem, this, menu_selector(GDRTOverlay::onLauncher)
                );
                launcher->setPosition({win.width - 52.f, win.height * 0.52f});
                launcherMenu->addChild(launcher);
            }
            addChild(launcherMenu, 1000);

            constexpr float panelWidth = 360.f;
            constexpr float panelHeight = 340.f;
            m_panelScale = std::min({1.f, (win.width - 24.f) / panelWidth, (win.height - 24.f) / panelHeight});
            m_panel = CCLayer::create();
            m_panel->setContentSize({panelWidth, panelHeight});
            m_panel->setPosition({(win.width - panelWidth * m_panelScale) / 2.f,
                                  (win.height - panelHeight * m_panelScale) / 2.f});
            m_panel->setScale(0.01f * m_panelScale);
            m_panel->setVisible(false);
            addChild(m_panel, 900);

            if (auto background = CCSprite::create("Menu.png"_spr)) {
                auto size = background->getContentSize();
                background->setPosition({panelWidth / 2.f, panelHeight / 2.f});
                background->setScaleX(panelWidth / size.width);
                background->setScaleY(panelHeight / size.height);
                m_panel->addChild(background, -10);
            }

            auto title = CCLabelBMFont::create("GD REPLAY TOOLS", "bigFont.fnt");
            title->setScale(0.62f);
            title->setPosition({panelWidth / 2.f, 316.f});
            m_panel->addChild(title, 5);

            auto tabs = CCMenu::create();
            tabs->setPosition(CCPointZero);
            m_panel->addChild(tabs, 10);
            auto toolsTab = makeButton("TOOLS", "GJ_button_04.png", [this] { showTools(); });
            toolsTab->setPosition({105.f, 278.f});
            tabs->addChild(toolsTab);
            auto recordTab = makeButton("RECORD / PLAY", "GJ_button_04.png", [this] { showRecord(); });
            recordTab->setPosition({255.f, 278.f});
            tabs->addChild(recordTab);

            m_toolsPage = CCLayer::create();
            m_toolsPage->setContentSize({panelWidth, 250.f});
            m_toolsPage->setPosition({0.f, 0.f});
            m_panel->addChild(m_toolsPage, 2);

            auto toolsMenu = CCMenu::create();
            toolsMenu->setPosition(CCPointZero);
            m_toolsPage->addChild(toolsMenu);

            auto noclip = makeButton("NOCLIP", "GJ_button_04.png", [this] { toggleNoclip(); });
            noclip->setPosition({180.f, 218.f});
            toolsMenu->addChild(noclip);

            m_noclipStatus = CCLabelBMFont::create("", "bigFont.fnt");
            m_noclipStatus->setScale(0.58f);
            m_noclipStatus->setPosition({180.f, 178.f});
            m_toolsPage->addChild(m_noclipStatus);

            auto slower = makeButton("-", "GJ_button_01.png", [this] { changeSpeed(-0.25f); });
            slower->setPosition({85.f, 100.f});
            toolsMenu->addChild(slower);
            auto faster = makeButton("+", "GJ_button_01.png", [this] { changeSpeed(0.25f); });
            faster->setPosition({275.f, 100.f});
            toolsMenu->addChild(faster);
            auto speedToggle = makeButton("SPEEDHACK", "GJ_button_04.png", [this] { toggleSpeed(); });
            speedToggle->setPosition({180.f, 145.f});
            toolsMenu->addChild(speedToggle);

            m_speedStatus = CCLabelBMFont::create("", "chatFont.fnt");
            m_speedStatus->setScale(0.72f);
            m_speedStatus->setPosition({180.f, 65.f});
            m_toolsPage->addChild(m_speedStatus);

            m_recordPage = CCLayer::create();
            m_recordPage->setContentSize({panelWidth, 250.f});
            m_recordPage->setPosition({0.f, 0.f});
            m_panel->addChild(m_recordPage, 2);
            auto recordMenu = CCMenu::create();
            recordMenu->setPosition(CCPointZero);
            m_recordPage->addChild(recordMenu);

            auto record = makeButton("RECORD", "GJ_button_04.png", [this] {
                if (gdrt::isRecording()) gdrt::stopRecording();
                else gdrt::startRecording();
                refreshLabels();
            });
            record->setPosition({100.f, 210.f});
            recordMenu->addChild(record);

            auto save = makeButton("SAVE", "GJ_button_01.png", [this] {
                gdrt::saveRecording();
                refreshLabels();
            });
            save->setPosition({260.f, 210.f});
            recordMenu->addChild(save);

            auto play = makeButton("PLAY", "GJ_button_04.png", [] { gdrt::startPlayback(); });
            play->setPosition({180.f, 150.f});
            recordMenu->addChild(play);

            auto createTemplate = makeButton("CREATE TEMPLATE", "GJ_button_04.png", [this] {
                gdrt::createTemplate();
                refreshLabels();
            });
            createTemplate->setPosition({105.f, 85.f});
            recordMenu->addChild(createTemplate);

            auto deleteTemplate = makeButton("DELETE TEMPLATE", "GJ_button_02.png", [this] {
                gdrt::deleteTemplate();
                refreshLabels();
            });
            deleteTemplate->setPosition({255.f, 85.f});
            recordMenu->addChild(deleteTemplate);

            m_recordStatus = CCLabelBMFont::create("", "chatFont.fnt");
            m_recordStatus->setScale(0.7f);
            m_recordStatus->setPosition({180.f, 180.f});
            m_recordPage->addChild(m_recordStatus);

            m_templateStatus = CCLabelBMFont::create("", "chatFont.fnt");
            m_templateStatus->setScale(0.7f);
            m_templateStatus->setPosition({180.f, 45.f});
            m_recordPage->addChild(m_templateStatus);

            showTools();
            return true;
        }

        void onLauncher(CCObject*) { togglePanel(); }

        void togglePanel() {
            if (!m_panel) return;
            if (m_panel->isVisible()) { hidePanel(); return; }
            m_panel->stopAllActions();
            m_panel->setVisible(true);
            m_panel->setScale(0.01f * m_panelScale);
            m_panel->runAction(CCEaseBackOut::create(CCScaleTo::create(0.22f, m_panelScale)));
        }

        void hidePanel() {
            if (!m_panel || !m_panel->isVisible()) return;
            m_panel->stopAllActions();
            m_panel->runAction(CCSequence::create(
                CCEaseBackIn::create(CCScaleTo::create(0.16f, 0.01f * m_panelScale)),
                CCCallFunc::create(this, callfunc_selector(GDRTOverlay::finishHide)),
                nullptr
            ));
        }

        void finishHide() { if (m_panel) m_panel->setVisible(false); }

    public:
        static GDRTOverlay* create() {
            auto ret = new GDRTOverlay();
            if (ret && ret->init()) { ret->autorelease(); return ret; }
            CC_SAFE_DELETE(ret);
            return nullptr;
        }
    };

    void addOverlay(CCNode* layer) {
        if (layer) {
            if (auto overlay = GDRTOverlay::create()) layer->addChild(overlay, 10000);
        }
    }
}

class $modify(GDRTMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        addOverlay(this);
        return true;
    }
};

class $modify(GDRTGameplayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        s_noclipEnabled = Mod::get()->getSettingValue<bool>("noclip-enabled");
        s_speedEnabled = Mod::get()->getSettingValue<bool>("speedhack-enabled");
        addOverlay(this);
        return true;
    }

    void update(float dt) {
        float multiplier = s_speedEnabled
            ? Mod::get()->getSettingValue<float>("speedhack-multiplier")
            : 1.f;
        float scaledDelta = dt * multiplier;
        PlayLayer::update(scaledDelta);
        gdrt::advancePlayback(m_player1, scaledDelta);
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (s_noclipEnabled) return;
        PlayLayer::destroyPlayer(player, object);
    }

    void onQuit() {
        s_noclipEnabled = false;
        s_speedEnabled = false;
        Mod::get()->setSettingValue("noclip-enabled", false);
        Mod::get()->setSettingValue("speedhack-enabled", false);
        gdrt::stopPlayback();
        PlayLayer::onQuit();
    }
};

$on_mod(Loaded) {
    s_noclipEnabled = Mod::get()->getSettingValue<bool>("noclip-enabled");
    s_speedEnabled = Mod::get()->getSettingValue<bool>("speedhack-enabled");
    log::info("GD Replay Tools loaded.");
}
