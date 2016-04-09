#include "CarrotCanvas.h"

CarrotCanvas::CarrotCanvas(QWidget* parent, const QPoint& position, const QSize& size) :
        QSFMLCanvas(parent, position, size) {

}

void CarrotCanvas::onInit() {
}

void CarrotCanvas::onUpdate() {
}

void CarrotCanvas::updateContents() {
    repaint();
}
