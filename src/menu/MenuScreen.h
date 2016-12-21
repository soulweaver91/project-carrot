#pragma once

#include <memory>
#include <functional>
#include <QVector>
#include <QKeyEvent>
#include <QString>
#include <SFML/Graphics.hpp>

#include "../EngineState.h"
#include "../CarrotQt5.h"
#include "../graphics/BitmapString.h"

class MenuScreen;
class CarrotQt5;

typedef std::function<void()> MenuFunctionCallback;

struct MenuItem {
    MenuFunctionCallback callback;
    std::unique_ptr<BitmapString> text;
};

class MenuScreen : public EngineState {
public:
    MenuScreen(CarrotQt5* root);
    ~MenuScreen();

    virtual void logicTick(const ControlEventList& events) override;
    virtual void renderTick(bool topmost, bool topmostAfterPause) override;
    QString getType() override;

    void processControlEvents(const ControlEventList& events);

protected:
    void clearMenuList();
    void setMenuItemSelected(int idx = 0, bool relative = false);
    virtual void processControlDownEvent(const ControlEvent& e);
    virtual void processControlHeldEvent(const ControlEvent& e);
    virtual void processControlUpEvent(const ControlEvent& e);

    std::shared_ptr<MenuItem> buildMenuItem(MenuFunctionCallback callback, const QString& label);

    CarrotQt5* root;
    QVector<std::shared_ptr<MenuItem>> menuOptions;
    std::shared_ptr<MenuItem> cancelItem;
    int selectedItemIdx;
    BitmapString attractionText;

    static const MenuFunctionCallback placeholderOption;
};
