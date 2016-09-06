#pragma once

#include <memory>
#include <QString>
#include <QVector>
#include <SFML/Graphics.hpp>

class BitmapFont {
    public:
        BitmapFont(const QString& filename, unsigned width, unsigned height, unsigned cols, unsigned first, unsigned last);
        std::shared_ptr<sf::Sprite> getCharacterSprite(QChar code);
        unsigned getCharacterWidth(QChar code);
    private:
        short charWidth[256];
        sf::Texture fontTexture;
        QVector<std::shared_ptr<sf::Sprite>> characterMap;
        short firstCharCode;
};
