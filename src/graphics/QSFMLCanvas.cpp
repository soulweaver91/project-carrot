#include "QSFMLCanvas.h"
#include "SFML/Window.hpp"

QSFMLCanvas::QSFMLCanvas(QWidget* parent, const QPoint& position, const QSize& size) :
                    QWidget(parent), initialized(false) {
    // Setup some states to allow direct rendering into the widget
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);

    // Set strong focus to enable keyboard events to be received
    setFocusPolicy(Qt::StrongFocus);

    // Setup the widget geometry
    move(position);
    resize(size);
}

#ifdef Q_WS_X11
    #include <Qt/qx11info_x11.h>
    #include <X11/Xlib.h>
#endif

void QSFMLCanvas::showEvent(QShowEvent*) {
    if (!initialized) {
        // Under X11, we need to flush the commands sent to the server to ensure that
        // SFML will get an updated view of the windows
        #ifdef Q_WS_X11
            XFlush(QX11Info::display());
        #endif

        sf::RenderWindow::create((sf::WindowHandle)winId());

        // Let the derived class do its specific stuff
        onInit();
        initialized = true;
    }
}

QSFMLCanvas::~QSFMLCanvas() {

}

void QSFMLCanvas::onInit() {

}

void QSFMLCanvas::onUpdate() {

}

QPaintEngine* QSFMLCanvas::paintEngine() const {
    return 0;
}

void QSFMLCanvas::paintEvent(QPaintEvent*) {
    // Let the derived class do its specific stuff
    onUpdate();

    // Display on screen
    display();
}
