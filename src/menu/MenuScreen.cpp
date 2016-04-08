#include <memory>
#include "MenuScreen.h"
#include <QDir>
#include <QSettings>

MenuScreen::MenuScreen(std::shared_ptr<CarrotQt5> root, MenuEntryPoint entry) : root(root), selected_item(0),
    current_type(MENU_PLAIN_LIST), attraction_text(root->getFont(), "", FONT_ALIGN_RIGHT) {
    glow_a_tex.loadFromFile("Data/Textures/radialglow.png");
    glow_a.setTexture(glow_a_tex);
    glow_a.setPosition(400,100);
    glow_a.setOrigin(313,313);
    
    glow_b_tex.loadFromFile("Data/Textures/coneglow.png");
    for (int i = 0; i < 4; ++i) {
        glow_b[i].setTexture(glow_b_tex);
        glow_b[i].setPosition(400,100);
        glow_b[i].setOrigin(121,78);
        glow_b[i].setRotation(i * 90);
    }
    
    logo_tex.loadFromFile("Data/logo-300px.png");
    logo.setTexture(logo_tex);
    logo.setPosition(400,10);
    logo.setOrigin(150,0);

    cancel_item = buildMenuItem(&MenuScreen::placeholderOption,QVariant(""),"");
    switch (entry) {
        case MENU_MAIN_MENU:
            loadMainMenu(QVariant(""));
            break;
        case MENU_PAUSE_MENU:
            break;
    }

}

MenuScreen::~MenuScreen() {

}

void MenuScreen::clearMenuList() {
    menu_options.clear();
    selected_item = 0;
}

std::shared_ptr<MenuItem> MenuScreen::buildMenuItem(InvokableMenuFunction local_func, QVariant param, const QString& label) {
    auto m = std::make_shared<MenuItem>();
    m->is_local = true;
    m->local_function = local_func;
    m->param = param;
    m->text = std::make_unique<BitmapString>(root->getFont(), label, FONT_ALIGN_CENTER);
    return m;
}

std::shared_ptr<MenuItem> MenuScreen::buildMenuItem(InvokableRootFunction remote_func, QVariant param, const QString& label) {
    auto m = std::make_shared<MenuItem>();
    m->is_local = false;
    m->remote_function = remote_func;
    m->param = param;
    m->text = std::make_unique<BitmapString>(root->getFont(), label, FONT_ALIGN_CENTER);
    return m;
}

void MenuScreen::setMenuItemSelected(int idx, bool relative) {
    if (menu_options.size() == 0) {
        // this should not happen
        return;
    }
    
    menu_options[selected_item]->text->setAnimation(false);
    menu_options[selected_item]->text->setColoured(false);

    int new_idx = (relative ? selected_item : 0) + idx;
    while (new_idx < 0) {
        new_idx += menu_options.size();
    }
    while (new_idx >= menu_options.size()) {
        new_idx -= menu_options.size();
    }
    selected_item = new_idx;
    menu_options[selected_item]->text->setAnimation(true,4,4,0.05,0.3);
    menu_options[selected_item]->text->setColoured(true);
}

void MenuScreen::processControlDownEvent(const ControlEvent& e) {
    std::shared_ptr<MenuItem> it;
    switch (e.first.keyboardKey) {
        case Qt::Key_Escape:
            it = cancel_item;
            if (it->is_local) {
                (this->*(it->local_function))(it->param);
            } else {
                root->invokeFunction(it->remote_function,it->param);
            }
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            // Select the currently highlighted option and run its designated function
            it = menu_options[selected_item];
            if (it->is_local) {
                (this->*(it->local_function))(it->param);
            } else {
                root->invokeFunction(it->remote_function,it->param);
            }
            break;
        case Qt::Key_Up:
            // Move selection up
            setMenuItemSelected(-1,true);
            break;
        case Qt::Key_Down:
            // Move selection down
            setMenuItemSelected(1,true);
            break;
    }
}

void MenuScreen::processControlHeldEvent(const ControlEvent& e) {
    if (e.second.heldDuration > 20 && e.second.heldDuration % 5 == 0) {
        processControlDownEvent(e);
    }
}

void MenuScreen::processControlUpEvent(const ControlEvent& e) {
    // No use at the moment, but defined for the sake of consistency.
}

void MenuScreen::tickEvent() {
    auto canvas = root->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

    unsigned int view_w = root->getViewWidth();
    unsigned int view_h = root->getViewHeight();

    for (int i = 0; i < 4; ++i) {
        glow_b[i].rotate(0.4);
        glow_b[i].setPosition(sf::Vector2f(view_w/2,100));
        canvas->draw(glow_b[i]);
    }
    glow_a.setPosition(sf::Vector2f(view_w/2,100));
    logo.setPosition(sf::Vector2f(view_w/2,10));
    canvas->draw(glow_a);
    canvas->draw(logo);
    

    BitmapString::drawString(canvas,root->getFont(), CP_VERSION + " v" + QString::number(CP_VERSION_NUM), 10,view_h - 30);
    attraction_text.drawString(canvas,view_w-10,view_h-30);

    switch (current_type) {
        case MENU_PLAIN_LIST:
            if (menu_options.size() < 10) {
                for (int i = 0; i < menu_options.size(); ++i) {
                    menu_options[i]->text->drawString(canvas,view_w / 2,200 + ((view_h - 280) / menu_options.size())*i);
                }
            } else {
                int j = 0 - std::min(0, selected_item - 5);
                for (int i = std::max(0, selected_item - 5); i < std::min(menu_options.size(), selected_item + 6); ++i, ++j) {
                    menu_options[i]->text->drawString(canvas,view_w / 2,226 + 26*j);
                }
                if (selected_item > 5) {
                    BitmapString::drawString(canvas, root->getFont(), "-=...=-", view_w / 2 - 40, 200);
                }
                if ((menu_options.size() - selected_item - 1) > 5) {
                    BitmapString::drawString(canvas, root->getFont(), "-=...=-", view_w / 2 - 40, 512);
                }
            }
            break;
        default:
            // ?
            break;
    }
}

void MenuScreen::loadLevelList(QVariant param) {
    clearMenuList();
    QDir level_dir("Levels");
    if (level_dir.exists()) {
        QStringList levels = level_dir.entryList();
        for (int i = 0; i < levels.size(); ++i) {
            if (levels.at(i) == "." || levels.at(i) == "..") {
                continue;
            }
            if (QDir(level_dir.absoluteFilePath(levels.at(i))).exists()) {
                QSettings level_data(level_dir.absoluteFilePath(levels.at(i)) + "/config.ini",QSettings::Format::IniFormat);
                menu_options.append(buildMenuItem(&CarrotQt5::startGame,QVariant(level_dir.absoluteFilePath(levels.at(i))),level_data.value("Level/FormalName").toString() + " ~ " + levels.at(i)));
            }
        }
    } 
    menu_options.append(buildMenuItem(&MenuScreen::loadMainMenu,QVariant(""),"Cancel"));
    setMenuItemSelected(0);
    cancel_item->local_function = &MenuScreen::loadMainMenu;
    current_type = MENU_PLAIN_LIST;
    attraction_text.setText("Select Level");
}

void MenuScreen::loadEpisodeList(QVariant param) {
    clearMenuList();
    QDir ep_dir("Episodes");
    if (ep_dir.exists()) {
        QStringList eps = ep_dir.entryList();
        for (int i = 0; i < eps.size(); ++i) {
            if (eps.at(i) == "." || eps.at(i) == "..") {
                continue;
            }
            if (QDir(ep_dir.absoluteFilePath(eps.at(i))).exists()) {
                QSettings level_data(ep_dir.absoluteFilePath(eps.at(i)) + "/config.ini",QSettings::Format::IniFormat);
                menu_options.append(buildMenuItem(&CarrotQt5::startGame,
                    QVariant(ep_dir.absoluteFilePath(eps.at(i)) + "/" + level_data.value("Episode/FirstLevel").toString()),
                    level_data.value("Episode/FormalName").toString()));
            }
        }
    } 
    menu_options.append(buildMenuItem(&MenuScreen::loadLevelList,QVariant(""),"Home Cooked Levels"));
    setMenuItemSelected(0);
    cancel_item->local_function = &MenuScreen::loadMainMenu;
    current_type = MENU_PLAIN_LIST;
    attraction_text.setText("Select Episode");
}

void MenuScreen::loadMainMenu(QVariant param) {
    clearMenuList();
    
    menu_options.append(buildMenuItem(&MenuScreen::loadEpisodeList,QVariant(""),"New Game"));
    menu_options.append(buildMenuItem(&MenuScreen::placeholderOption,QVariant(""),"Load Game"));
    menu_options.append(buildMenuItem(&MenuScreen::placeholderOption,QVariant(""),"Settings"));
    menu_options.append(buildMenuItem(&MenuScreen::placeholderOption,QVariant(""),"High Scores"));
    menu_options.append(buildMenuItem(&CarrotQt5::quitFromMainMenu,QVariant(""),"Quit Game"));
    setMenuItemSelected(0);
    cancel_item->local_function = &MenuScreen::placeholderOption;
    current_type = MENU_PLAIN_LIST;
    attraction_text.setText("Main Menu");
}

void MenuScreen::placeholderOption(QVariant param) {
    // do nothing
}
