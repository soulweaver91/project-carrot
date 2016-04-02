#pragma once

#include <QWidget>
#include <QPoint>
#include <QSize>
#include <QPaintEngine>
#include <QShowEvent>
#include <QPaintEvent>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

class QSFMLCanvas : public QWidget, public sf::RenderWindow {
    public:
        QSFMLCanvas(QWidget* parent, const QPoint& position, const QSize& size);
        virtual ~QSFMLCanvas();

    private:
        virtual void OnInit();
        virtual void OnUpdate();
        virtual QPaintEngine* paintEngine() const;
        virtual void showEvent(QShowEvent*);
        virtual void paintEvent(QPaintEvent*);

        bool initialized;
};
