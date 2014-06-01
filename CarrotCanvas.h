#ifndef CARROTCANVAS_H
#define CARROTCANVAS_H

#include "QSFMLCanvas.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <QWidget>
#include <QTimer>

class CarrotCanvas : public QSFMLCanvas
{
    public:
        CarrotCanvas(QWidget* Parent, const QPoint& Position, const QSize& Size);
        void updateContents();
    
    private:
        void OnInit();
        void OnUpdate();
};

#endif
