#include "CarrotCanvas.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <QWidget>
#include <QTimer>

CarrotCanvas::CarrotCanvas(QWidget* Parent, const QPoint& Position, const QSize& Size) :
        QSFMLCanvas(Parent, Position, Size) {

}

void CarrotCanvas::OnInit() {
}

void CarrotCanvas::OnUpdate() {
}

void CarrotCanvas::updateContents() {
    repaint();
    //display();
}
