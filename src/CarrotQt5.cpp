#include "CarrotQt5.h"
#include "gamestate/LevelManager.h"
#include "graphics/QSFMLCanvas.h"
#include "graphics/CarrotCanvas.h"
#include "menu/MenuScreen.h"
#include "struct/Constants.h"
#include "JJ2Format.h"

#include "ui_AboutCarrotDialog.h"

#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QCloseEvent>
#include <QTimer>
#include <QTime>
#include <QDesktopServices>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QTransform>
#include <QStringList>
#include <exception>
#include <bass.h>

CarrotQt5::CarrotQt5(QWidget *parent) : QMainWindow(parent), initialized(false), paused(false),
    frame(0), menuObject(nullptr), isMenu(false), fps(0) {
#ifndef CARROT_DEBUG
    // Set application location as the working directory
    QDir::setCurrent(QCoreApplication::applicationDirPath());
#endif

    // Apply the UI file, set window flags and connect menu items to class slots
    ui.setupUi(this);
    //setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

    connect(ui.action_About_Project_Carrot, SIGNAL(triggered()), this, SLOT(openAboutCarrot()));
    connect(ui.action_About_Qt_5, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui.action_Home_Page, SIGNAL(triggered()), this, SLOT(openHomePage()));

    afterTickCallback = []() {};

#ifdef CARROT_DEBUG
    debugConfig.dbgOverlaysActive = true;
    debugConfig.dbgShowMasked = false;
    debugConfig.currentTempModifier = 0;
    for (int i = 0; i < 32; ++i) {
        debugConfig.tempModifierName[i] = "(UNSET)";
        debugConfig.tempModifier[i] = 0;
    }
    debugConfig.tempModifierName[0] = "PerspCurve";      debugConfig.tempModifier[0] = 20;
    debugConfig.tempModifierName[1] = "PerspMultipNear"; debugConfig.tempModifier[1] = 1;
    debugConfig.tempModifierName[2] = "PerspMultipFar";  debugConfig.tempModifier[2] = 3;
    debugConfig.tempModifierName[3] = "PerspSpeed";      debugConfig.tempModifier[3] = 4;
    debugConfig.tempModifierName[4] = "SkyDepth";        debugConfig.tempModifier[4] = 1;

    connect(ui.debug_music, SIGNAL(triggered()), this, SLOT(debugLoadMusic()));
    connect(ui.debug_masks, SIGNAL(triggered(bool)), this, SLOT(debugShowMasks(bool)));
    connect(ui.debug_overlays, SIGNAL(triggered(bool)), this, SLOT(debugSetOverlaysActive(bool)));
#else
    ui.menuDebug->menuAction()->setVisible(false);
#endif

    // Initialize the paint surface
    windowCanvas = std::make_shared<CarrotCanvas>(this, QPoint(0, 0), QSize(DEFAULT_RESOLUTION_W, DEFAULT_RESOLUTION_H));
    windowCanvas->show();

    // Resize the main window to exactly fit the menu bar and the game view itself.
    // There might be a less hacky way to do this, but if there is, it is not very obvious.
    setCentralWidget(windowCanvas.get());
    centralWidget()->setFixedSize(QSize(DEFAULT_RESOLUTION_W, DEFAULT_RESOLUTION_H));
    resize(sizeHint());
    centralWidget()->setMinimumSize(QSize(320, 200));
    centralWidget()->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

    resourceManager = std::make_unique<ResourceManager>();
    controlManager = std::make_shared<ControlManager>();

    bool shadersSupported = ShaderSource::initialize();
    int maxTextureSize = sf::Texture::getMaximumSize();
    if (!shadersSupported  || maxTextureSize < 4096) {
        QMessageBox::critical(nullptr, "System unsupported", QString(
            "Your graphics card is not compatible with Project Carrot. For running Project Carrot, "
            "support for OpenGL shaders and a minimum texture size of 4096 by 4096 pixels is required.\n\n"
            "Your graphics card does%1 support shaders.\n"
            "Your graphics card has a maximum texture size of %2 by %2 pixels."
        ).arg(shadersSupported ? "" : " not").arg(maxTextureSize));
        exit(EXIT_FAILURE);
        return;
    }

    // Read the main font
    smallFont = std::make_shared<BitmapFont>("Data/Assets/ui/font_small.png",  17, 18, 15, 32, 256, -2);
    mainFont  = std::make_shared<BitmapFont>("Data/Assets/ui/font_medium.png", 29, 31, 15, 32, 256, -1);
    largeFont = std::make_shared<BitmapFont>("Data/Assets/ui/font_large.png",  57, 63, 15, 32, 256, -1);
    
    installEventFilter(this);

    // Define pause screen resources
    pausedScreenshot = std::make_unique<sf::Texture>();
    pausedScreenshot->create(DEFAULT_RESOLUTION_W, DEFAULT_RESOLUTION_H);
    pausedScreenshot->update(*windowCanvas);

    pausedScreenshotSprite = std::make_unique<sf::Sprite>();
    pausedScreenshotSprite->setTexture(*pausedScreenshot);

    // Define the pause text and add vertical bounce animation to it
    pausedText = std::make_unique<BitmapString>(mainFont, "Pause", FONT_ALIGN_CENTER);
    pausedText->setAnimation(true, 0.0, 6.0, 0.015, 1.25);

    lastTimestamp = QTime::currentTime();

    myTimer.setInterval(1000 / 70);
    connect(&myTimer, SIGNAL(timeout()), this, SLOT(tick()));
    myTimer.start();

    initialized = true;
}

CarrotQt5::~CarrotQt5() {
    if (!initialized) {
        return;
    }

    ShaderSource::teardown();
}

void CarrotQt5::parseCommandLine() {
    if (!initialized) {
        return;
    }

    QStringList param = QCoreApplication::arguments();
    QString level = "";
    if (param.size() > 1) {
        QDir dir(QDir::currentPath());
        if (dir.exists(param.at(param.size() - 1))) {
            level = param.at(param.size() - 1);
        }
    }

    if (level != "") {
        startGame(level);
    } else {
        startMainMenu();
    }

    afterTickCallback();
}

void CarrotQt5::startGame(const QString& filename) {
    return startGame(filename, "");
}

void CarrotQt5::startGame(const QString& filename, const QString& episode, const LevelCarryOver carryOver) {
    afterTickCallback = [this, filename, episode, carryOver]() {
        resourceManager->getSoundSystem()->clearSounds();
        resourceManager->getSoundSystem()->unregisterAllSoundListeners();
        resourceManager->getGraphicsCache()->flush();

        try {
            windowCanvas->clear();

            levelManager = std::make_unique<LevelManager>(this, filename, episode);

#ifdef CARROT_DEBUG
            auto player = levelManager->getPlayer(0).lock();
            if (player != nullptr) {
                connect(ui.debug_health, SIGNAL(triggered()), (QObject*)player.get(), SLOT(debugHealth()));
                connect(ui.debug_ammo, SIGNAL(triggered()), (QObject*)player.get(), SLOT(debugAmmo()));
            }

            connect(ui.debug_gravity, SIGNAL(triggered()), (QObject*)levelManager.get(), SLOT(debugSetGravity()));
            connect(ui.debug_lighting, SIGNAL(triggered()), (QObject*)levelManager.get(), SLOT(debugSetLighting()));
            connect(ui.debug_position, SIGNAL(triggered()), (QObject*)levelManager.get(), SLOT(debugSetPosition()));
            connect(ui.debug_rush, SIGNAL(triggered()), (QObject*)levelManager.get(), SLOT(debugSugarRush()));

            ui.debug_overlays->setDisabled(false);
            ui.debug_masks->setDisabled(false);
            ui.debug_health->setDisabled(false);
            ui.debug_ammo->setDisabled(false);
            ui.debug_gravity->setDisabled(false);
            ui.debug_lighting->setDisabled(false);
            ui.debug_position->setDisabled(false);
            ui.debug_rush->setDisabled(false);
#endif

            levelManager->processCarryOver(carryOver);

            currentMode = levelManager.get();
            isMenu = false;
            menuObject = nullptr;
            afterTickCallback = []() {};
        } catch (const std::exception& ex) {
            QMessageBox::critical(this, "Error loading level", ex.what());

            if (!isMenu) {
                startMainMenu();
            }
        }
    };
}

void CarrotQt5::startMainMenu() {
    afterTickCallback = [this]() {
        menuObject = std::make_unique<MenuScreen>(this);
        setWindowTitle("Project Carrot");

        resourceManager->getSoundSystem()->clearSounds();
        resourceManager->getSoundSystem()->unregisterAllSoundListeners();
        resourceManager->getSoundSystem()->setMusic("Music/Menu.it");

        windowCanvas->clear();

#ifdef CARROT_DEBUG
        ui.debug_overlays->setDisabled(true);
        ui.debug_masks->setDisabled(true);
        ui.debug_health->setDisabled(true);
        ui.debug_ammo->setDisabled(true);
        ui.debug_gravity->setDisabled(true);
        ui.debug_lighting->setDisabled(true);
        ui.debug_position->setDisabled(true);
        ui.debug_rush->setDisabled(true);
#endif

        currentMode = menuObject.get();
        isMenu = true;
        levelManager = nullptr;

        afterTickCallback = []() {};
    };
}

void CarrotQt5::openAboutCarrot() {
    QDialog* dialog = new QDialog(this, 0);
    Ui_AboutCarrot ui;
    ui.setupUi(dialog);

    dialog->show();
}

void CarrotQt5::openHomePage() {
    QDesktopServices::openUrl(QUrl("https://carrot.soulweaver.fi/"));
}

/*void CarrotQt5::openHelp() {
    QDesktopServices::openUrl(QUrl("http://www.google.com/"));
}*/

void CarrotQt5::closeEvent(QCloseEvent* event) {
    QMessageBox msg(this);
    msg.setWindowTitle("Quit Project Carrot?");
    msg.setText("Are you sure you want to quit Project Carrot?");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg.setDefaultButton(QMessageBox::No);
    msg.setIcon(QMessageBox::Question);
    msg.setWindowModality(Qt::ApplicationModal);

    if (msg.exec() == QMessageBox::No) {
        event->ignore();
    }
}

bool CarrotQt5::eventFilter(QObject*, QEvent* e) {
    // Catch focus events to mute the music when the window doesn't have it
    if (e->type() == QEvent::WindowActivate) {
        if (!isMenu) {
            windowCanvas->draw(*pausedScreenshotSprite);
            windowCanvas->updateContents();
            pausedScreenshot->update(*windowCanvas);
        }
        paused = false;
        resourceManager->getSoundSystem()->fadeMusicIn(1000);
    } else if (e->type() == QEvent::WindowDeactivate) {
        resourceManager->getSoundSystem()->fadeMusicOut(1000);
        if (!paused) {
            pausedScreenshot->update(*windowCanvas);
        }
        paused = true;
    }
    return FALSE;  // dispatch normally
}

void CarrotQt5::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        return;
    }

    if (!isMenu) {
        if (event->key() == Qt::Key::Key_Escape) {
            startMainMenu();
        }

#ifdef CARROT_DEBUG
        if (event->key() == Qt::Key::Key_Insert) {
            debugConfig.tempModifier[debugConfig.currentTempModifier] += 1;
        }
        if (event->key() == Qt::Key::Key_Delete) {
            debugConfig.tempModifier[debugConfig.currentTempModifier] -= 1;
        }
        if (event->key() == Qt::Key::Key_PageUp) {
            debugConfig.currentTempModifier = (debugConfig.currentTempModifier + 1) % DEBUG_VARS_SIZE;
        }
        if (event->key() == Qt::Key::Key_PageDown) {
            debugConfig.currentTempModifier = (debugConfig.currentTempModifier + DEBUG_VARS_SIZE - 1) % DEBUG_VARS_SIZE;
        }
#endif
    }

    controlManager->setControlHeldDown(event->key());
}

void CarrotQt5::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        return;
    }

    controlManager->setControlReleased(event->key());
}

void CarrotQt5::resizeEvent(QResizeEvent*) {
    int w = centralWidget()->width();
    int h = centralWidget()->height();

    windowCanvas->setSize(sf::Vector2u(w, h));
    windowCanvas->setView(sf::View(sf::FloatRect(0, 0, w, h)));
    if (!isMenu && levelManager != nullptr) {
        levelManager->resizeEvent(w, h);
    }

    pausedScreenshot->create(w, h);
    pausedScreenshotSprite->setTextureRect(sf::IntRect(0, 0, w, h));
}

void CarrotQt5::tick() {
    frame++;
    if (frame % 20 == 0) {
        QTime now = QTime::currentTime();
        fps = (1000.0 / (lastTimestamp.msecsTo(now) / 20.0));
        lastTimestamp = now;
    }

    // Clear the drawing surface; we don't want to do this if we emulate the JJ2 behavior
    windowCanvas->clear();

    if (paused) {
        // Set up a partially translucent black overlay
        sf::Vector2u viewSize = windowCanvas->getSize();
        sf::RectangleShape overlay;
        overlay.setSize(sf::Vector2f(viewSize));
        overlay.setFillColor(sf::Color(0, 0, 0, 120));

        if (isMenu) {
            currentMode->tick({});
            windowCanvas->draw(overlay);
        } else {
            windowCanvas->draw(*pausedScreenshotSprite);
            windowCanvas->draw(overlay);
            pausedText->drawString(getCanvas(), viewSize.x / 2, viewSize.y / 2 - 20);
        }

    } else {

        auto events = controlManager->getPendingEvents();
        currentMode->tick(events);
        controlManager->processFrame();
    }

    // Update the drawn surface to the screen
    windowCanvas->updateContents();

    afterTickCallback();
}

#ifdef CARROT_DEBUG

DebugConfig CarrotQt5::getDebugConfig() {
    return debugConfig;
}

void CarrotQt5::debugShowMasks(bool show) {
    debugConfig.dbgShowMasked = show;
}

void CarrotQt5::debugLoadMusic() {
    QString filename = QFileDialog::getOpenFileName(this, "Load which file?", qApp->applicationDirPath(),
        "Module files (*.it *.s3m *.xm *.mod *.mo3 *.mtm *.umx);;All files (*.*)");
    if (filename.endsWith(".j2b")) {
        filename = JJ2Format::convertJ2B(filename);
    }
    if (filename != "") {
        resourceManager->getSoundSystem()->setMusic(filename);
    }
}

void CarrotQt5::debugSetOverlaysActive(bool active) {
    debugConfig.dbgOverlaysActive = active;
}

#endif

unsigned long CarrotQt5::getFrame() {
    return frame;
}

std::shared_ptr<ResourceSet> CarrotQt5::loadActorTypeResources(const QString& actorType) {
    return resourceManager->loadActorTypeResources(actorType);
}

float CarrotQt5::getCurrentFPS() {
    return fps;
}

void CarrotQt5::quitFromMainMenu() {
    afterTickCallback = [this]() {
        close();
        afterTickCallback = []() {};
    };
}

CarrotCanvas* CarrotQt5::getCanvas() {
    return windowCanvas.get();
}

std::shared_ptr<BitmapFont> CarrotQt5::getFont(BitmapFontSize size) {
    switch (size) {
        case LARGE:
            return largeFont;
            break;
        case NORMAL:
            return mainFont;
            break;
        case SMALL:
            return smallFont;
            break;
    }
    
    return nullptr;
}

SoundSystem* CarrotQt5::getSoundSystem() {
    return resourceManager->getSoundSystem().get();
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    std::shared_ptr<CarrotQt5> w = std::make_shared<CarrotQt5>();
    w->show();
    w->parseCommandLine();
    return a.exec();
}
