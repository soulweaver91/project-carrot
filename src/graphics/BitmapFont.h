#pragma once

#include <memory>
#include <QString>
#include <QVector>
#include <SFML/Graphics.hpp>

class BitmapFont {
    public:
        BitmapFont(const QString& filename, unsigned width, unsigned height, unsigned cols, unsigned first, unsigned last, int defaultSpacing);
        std::shared_ptr<sf::Sprite> getCharacterSprite(QChar code);
        unsigned getCharacterWidth(QChar code);
        short getLineHeight();
        int getDefaultSpacing();
    private:
        short charWidth[256];
        short charHeight;
        sf::Texture fontTexture;
        QVector<std::shared_ptr<sf::Sprite>> characterMap;
        short firstCharCode;
        int defaultSpacing;
};
