#include "CarrotCanvas.h"

CarrotCanvas::CarrotCanvas(QWidget* Parent, const QPoint& Position, const QSize& Size) :
        QSFMLCanvas(Parent, Position, Size) {

}

void CarrotCanvas::OnInit() {
}

void CarrotCanvas::OnUpdate() {
}

void CarrotCanvas::updateContents() {
    repaint();
}
