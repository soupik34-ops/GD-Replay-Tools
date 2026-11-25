#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace {
    // Process-local by design: Kill Menu remains active until Geometry Dash restarts.
    bool s_menuKilled = false;

    class GDRTOverlay final : public CCLayer {
        CCNode* m_panel = nullptr;

        bool init() {
            if (!CCLayer::init()) return false;
            auto winSize = CCDirector::get()->getWinSize();

            auto floatingMenu = CCMenu::create();
            floatingMenu->setPosition(CCPointZero);
            auto launcher = CCMenuItemExt::createSpriteExtra(
                CircleButtonSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png", 1.f, CircleBaseColor::Cyan),
                [this](CCMenuItemSpriteExtra*) { this->togglePanel(); }
            );
            launcher->setPosition({winSize.width - 34.f, winSize.height * 0.55f});
            floatingMenu->addChild(launcher);
            this->addChild(floatingMenu, 20);

            auto panel = CCLayerColor::create({18, 25, 43, 238}, 230.f, 190.f);
            panel->setPosition({(winSize.width - 230.f) / 2.f, (winSize.height - 190.f) / 2.f});
            panel->setScale(0.01f);
            panel->setVisible(false);
            m_panel = panel;
            this->addChild(panel, 10);

            auto title = CCLabelBMFont::create("GD REPLAY TOOLS", "bigFont.fnt");
            title->setScale(0.42f);
            title->setPosition({115.f, 163.f});
            panel->addChild(title);

            auto menu = CCMenu::create();
            menu->setPosition(CCPointZero);
            panel->addChild(menu);

            auto hideButton = CCMenuItemExt::createSpriteExtra(
                ButtonSprite::create("Hide Menu", 90, 0, 0.55f, false, "goldFont.fnt", "Button04.png", 25.f),
                [this](CCMenuItemSpriteExtra*) { this->hidePanel(); }
            );
            hideButton->setPosition({115.f, 105.f});
            menu->addChild(hideButton);

            auto killButton = CCMenuItemExt::createSpriteExtra(
                ButtonSprite::create("Kill Menu", 90, 0, 0.55f, false, "goldFont.fnt", "Button05.png", 25.f),
                [this](CCMenuItemSpriteExtra*) { this->killMenu(); }
            );
            killButton->setPosition({115.f, 55.f});
            menu->addChild(killButton);
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

        void finishHide() {
            if (m_panel) m_panel->setVisible(false);
        }

        void killMenu() {
            s_menuKilled = true;
            this->removeFromParentAndCleanup(true);
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
        if (s_menuKilled || !layer) return;
        if (auto overlay = GDRTOverlay::create()) layer->addChild(overlay, 1000);
    }
}

class $modify(GDRTMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        addOverlay(this);
        return true;
    }
};

class $modify(GDRTPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        addOverlay(this);
        return true;
    }
};

$on_mod(Loaded) {
    log::info("GD Replay Tools overlay loaded.");
}
