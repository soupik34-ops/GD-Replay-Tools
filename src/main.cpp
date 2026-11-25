#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace {
    bool s_menuKilled = false;
    bool s_noclipEnabled = false;
    bool s_speedEnabled = false;

    void applySpeed() {
        auto director = CCDirector::get();
        if (!director || !director->getScheduler()) return;
        float multiplier = Mod::get()->getSettingValue<float>("speedhack-multiplier");
        director->getScheduler()->setTimeScale(s_speedEnabled ? multiplier : 1.f);
    }

    class GDRTOverlay final : public CCLayer {
        CCNode* m_panel = nullptr;
        CCLabelBMFont* m_noclipLabel = nullptr;
        CCLabelBMFont* m_speedLabel = nullptr;
        CCLabelBMFont* m_recordLabel = nullptr;

        void refreshLabels() {
            if (m_noclipLabel) m_noclipLabel->setString(s_noclipEnabled ? "NOCLIP: ON" : "NOCLIP: OFF");
            if (m_speedLabel) m_speedLabel->setString(s_speedEnabled ? "SPEED: ON" : "SPEED: OFF");
            if (m_recordLabel) {
                bool recording = Mod::get()->getSettingValue<bool>("record-inputs");
                m_recordLabel->setString(recording ? "RECORD INPUTS: ON" : "RECORD INPUTS: OFF");
            }
        }

        CCMenuItemSpriteExtra* makeButton(const char* text, const char* frame, std::function<void()> callback) {
            auto sprite = ButtonSprite::create(text, 118, 0, 0.55f, false, "goldFont.fnt", frame, 28.f);
            return CCMenuItemExt::createSpriteExtra(sprite, [callback](CCMenuItemSpriteExtra*) { callback(); });
        }

        bool init() {
            if (!CCLayer::init()) return false;
            auto win = CCDirector::get()->getWinSize();

            auto launcherMenu = CCMenu::create();
            launcherMenu->setPosition(CCPointZero);
            auto launcher = CCMenuItemExt::createSpriteExtra(
                CircleButtonSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png", 1.f, CircleBaseColor::Cyan),
                [this](CCMenuItemSpriteExtra*) { togglePanel(); }
            );
            launcher->setPosition({win.width - 36.f, win.height * 0.55f});
            launcherMenu->addChild(launcher);
            addChild(launcherMenu, 30);

            auto panel = CCLayerColor::create({13, 22, 42, 245}, 270.f, 250.f);
            panel->setPosition({(win.width - 270.f) / 2.f, (win.height - 250.f) / 2.f});
            panel->setScale(0.01f);
            panel->setVisible(false);
            m_panel = panel;
            addChild(panel, 20);

            // Native Geometry Dash panel texture, with a color fallback if the frame is unavailable.
            if (auto frame = CCSprite::createWithSpriteFrameName("GJ_square01.png")) {
                frame->setPosition({135.f, 125.f});
                frame->setScaleX(270.f / frame->getContentSize().width);
                frame->setScaleY(250.f / frame->getContentSize().height);
                frame->setColor({22, 48, 83});
                frame->setOpacity(235);
                panel->addChild(frame, -1);
            }

            auto title = CCLabelBMFont::create("GD REPLAY TOOLS", "bigFont.fnt");
            title->setScale(0.48f);
            title->setPosition({135.f, 220.f});
            panel->addChild(title);

            auto menu = CCMenu::create();
            menu->setPosition(CCPointZero);
            panel->addChild(menu);

            auto noclip = makeButton("NOCLIP", "GJ_button_04.png", [this] {
                s_noclipEnabled = !s_noclipEnabled;
                refreshLabels();
            });
            noclip->setPosition({75.f, 174.f});
            menu->addChild(noclip);

            auto speed = makeButton("SPEED", "GJ_button_04.png", [this] {
                s_speedEnabled = !s_speedEnabled;
                applySpeed();
                refreshLabels();
            });
            speed->setPosition({195.f, 174.f});
            menu->addChild(speed);

            auto record = makeButton("RECORD", "GJ_button_04.png", [this] {
                bool enabled = Mod::get()->getSettingValue<bool>("record-inputs");
                Mod::get()->setSettingValue("record-inputs", !enabled);
                refreshLabels();
            });
            record->setPosition({135.f, 124.f});
            menu->addChild(record);

            auto hide = makeButton("HIDE MENU", "GJ_button_01.png", [this] { hidePanel(); });
            hide->setPosition({75.f, 62.f});
            menu->addChild(hide);

            auto kill = makeButton("KILL MENU", "GJ_button_02.png", [this] { killMenu(); });
            kill->setPosition({195.f, 62.f});
            menu->addChild(kill);

            m_noclipLabel = CCLabelBMFont::create("", "chatFont.fnt");
            m_noclipLabel->setScale(0.55f);
            m_noclipLabel->setPosition({75.f, 148.f});
            panel->addChild(m_noclipLabel);

            m_speedLabel = CCLabelBMFont::create("", "chatFont.fnt");
            m_speedLabel->setScale(0.55f);
            m_speedLabel->setPosition({195.f, 148.f});
            panel->addChild(m_speedLabel);

            m_recordLabel = CCLabelBMFont::create("", "chatFont.fnt");
            m_recordLabel->setScale(0.55f);
            m_recordLabel->setPosition({135.f, 98.f});
            panel->addChild(m_recordLabel);
            refreshLabels();
            return true;
        }

        void togglePanel() {
            if (!m_panel) return;
            if (m_panel->isVisible()) { hidePanel(); return; }
            m_panel->stopAllActions();
            m_panel->setVisible(true);
            m_panel->setScale(0.01f);
            m_panel->runAction(CCEaseBackOut::create(CCScaleTo::create(0.22f, 1.f)));
        }

        void hidePanel() {
            if (!m_panel || !m_panel->isVisible()) return;
            m_panel->stopAllActions();
            m_panel->runAction(CCSequence::create(
                CCEaseBackIn::create(CCScaleTo::create(0.16f, 0.01f)),
                CCCallFunc::create(this, callfunc_selector(GDRTOverlay::finishHide)),
                nullptr
            ));
        }

        void finishHide() { if (m_panel) m_panel->setVisible(false); }

        void killMenu() {
            s_menuKilled = true;
            removeFromParentAndCleanup(true);
        }

    public:
        static GDRTOverlay* create() {
            auto ret = new GDRTOverlay();
            if (ret && ret->init()) { ret->autorelease(); return ret; }
            CC_SAFE_DELETE(ret);
            return nullptr;
        }
    };

    void addOverlay(CCNode* layer) {
        if (!s_menuKilled && layer) {
            if (auto overlay = GDRTOverlay::create()) layer->addChild(overlay, 1000);
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
        addOverlay(this);
        return true;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (s_noclipEnabled) return;
        PlayLayer::destroyPlayer(player, object);
    }

    void onQuit() {
        s_noclipEnabled = false;
        s_speedEnabled = false;
        applySpeed();
        PlayLayer::onQuit();
    }
};

$on_mod(Loaded) {
    log::info("GD Replay Tools loaded.");
}
