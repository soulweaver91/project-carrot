#pragma once

#include <QWidget>
#include <QPoint>
#include <QSize>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "QSFMLCanvas.h"

class CarrotCanvas : public QSFMLCanvas {
public:
    CarrotCanvas(QWidget* parent, const QPoint& position, const QSize& size);
    void updateContents();
    
private:
    void onInit();
    void onUpdate();
};
