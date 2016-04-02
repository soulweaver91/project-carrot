#pragma once

#include <QWidget>
#include <QPoint>
#include <QSize>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "QSFMLCanvas.h"

class CarrotCanvas : public QSFMLCanvas
{
    public:
        CarrotCanvas(QWidget* Parent, const QPoint& Position, const QSize& Size);
        void updateContents();
    
    private:
        void OnInit();
        void OnUpdate();
};
