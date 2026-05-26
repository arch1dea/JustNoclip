#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

using namespace geode::prelude;

static bool noclipEnabled = false;
static float rainbowHue = 0.f;
static CCMenuItemSpriteExtra* button = nullptr;
static CCLayer* menuLayer = nullptr;
static bool isMenuOpen = false;

class $modify(PlayerObject) {
    void deathState() {
        if (!noclipEnabled) {
            PlayerObject::deathState();
        }
    }
};

class FloatingButton : public CCLayer {
public:
    static FloatingButton* create() {
        auto ret = new FloatingButton();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init() override {
        auto winSize = CCDirector::get()->getWinSize();
        
        auto bg = CCSprite::create("square02b_001.png");
        bg->setScaleX(3.5f);
        bg->setScaleY(1.3f);
        bg->setColor({0, 0, 0});
        
        auto outline = CCScale9Sprite::create("square02b_001.png");
        outline->setScaleX(3.7f);
        outline->setScaleY(1.5f);
        outline->setZOrder(-1);
        bg->addChild(outline);
        outline->setPosition(bg->getContentSize() / 2);
        
        auto label = CCLabelBMFont::create("NO CLIP", "bigFont.fnt");
        label->setScale(0.7f);
        label->setPosition(bg->getContentSize() / 2);
        bg->addChild(label);
        
        auto menu = CCMenu::create();
        button = CCMenuItemSpriteExtra::create(bg, this, [](CCObject*) {
            noclipEnabled = !noclipEnabled;
            FLAlertLayer::create("NoClip", noclipEnabled ? "ON" : "OFF", "OK")->show();
            auto bgSprite = static_cast<CCSprite*>(button->getChildByIndex(0));
            if (bgSprite) {
                bgSprite->setColor(noclipEnabled ? ccColor3B{40,40,40} : ccColor3B{0,0,0});
            }
        });
        button->setPosition(winSize.width - 70, winSize.height - 50);
        menu->addChild(button);
        this->addChild(menu);
        
        this->schedule([this](float dt) {
            if (!button) return;
            rainbowHue += dt * 2.5f;
            if (rainbowHue > 1.f) rainbowHue -= 1.f;
            auto bg = static_cast<CCSprite*>(button->getChildByIndex(0));
            if (!bg) return;
            auto outline = static_cast<CCScale9Sprite*>(bg->getChildByIndex(0));
            if (outline) {
                auto color = geode::cocos::hsvToRgb({rainbowHue, 1.f, 1.f});
                outline->setColor({color.r, color.g, color.b});
            }
        }, "rainbow", 0.03f, false);
        
        return true;
    }
};

class NoClipMenu : public CCLayer {
public:
    static NoClipMenu* create() {
        auto ret = new NoClipMenu();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
    
    bool init() override {
        this->addChild(FloatingButton::create());
        auto winSize = CCDirector::get()->getWinSize();
        this->setPositionX(winSize.width);
        return true;
    }
    
    void openMenu() {
        if (isMenuOpen) return;
        isMenuOpen = true;
        auto winSize = CCDirector::get()->getWinSize();
        this->runAction(CCEaseBackOut::create(CCMoveTo::create(0.3f, ccp(0, 0))));
    }
    
    void closeMenu() {
        if (!isMenuOpen) return;
        isMenuOpen = false;
        auto winSize = CCDirector::get()->getWinSize();
        this->runAction(CCEaseBackIn::create(CCMoveTo::create(0.3f, ccp(winSize.width, 0))));
    }
    
    void toggleMenu() {
        if (isMenuOpen) closeMenu();
        else openMenu();
    }
};

class $modify(MenuLayer) {
    bool init() override {
        if (!MenuLayer::init()) return false;
        auto menu = NoClipMenu::create();
        menu->setTag(9999);
        this->addChild(menu);
        menuLayer = menu;
        return true;
    }
};

class $modify(CCKeyboardDispatcher) {
    void dispatchKeyboard(cocos2d::enumKeyCodes key, bool down, bool arr) {
        if (down && key == cocos2d::enumKeyCodes::KEY_J) {
            if (menuLayer) {
                static_cast<NoClipMenu*>(menuLayer)->toggleMenu();
            }
        }
        CCKeyboardDispatcher::dispatchKeyboard(key, down, arr);
    }
};
