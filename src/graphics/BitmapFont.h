#pragma once

#include <memory>
#include <QString>
#include <QVector>
#include <SFML/Graphics.hpp>

enum FontAlign {
    FONT_ALIGN_LEFT = 0,
    FONT_ALIGN_RIGHT = 1,
    FONT_ALIGN_CENTER = 2
};

class BitmapFont {
    public:
        BitmapFont(const QString& filename, unsigned width, unsigned height, unsigned rows, unsigned cols, unsigned first, unsigned last);
        std::shared_ptr<sf::Sprite> getCharacterSprite(QChar code);
        unsigned getCharacterWidth(QChar code);
    private:
        short charWidth[256];
        sf::Texture fontTexture;
        QVector<std::shared_ptr<sf::Sprite>> characterMap;
        short firstCharCode;
};

class BitmapString {
    public:
        BitmapString(std::shared_ptr<BitmapFont> font, const QString& initString = "", FontAlign initAlign = FONT_ALIGN_LEFT);
        void drawString(std::weak_ptr<sf::RenderWindow> destWindow, int x, int y);
        void setAnimation(bool setAnimated = false, double varianceX = 0.0, double varianceY = 0.0,
                          double animationSpeed = 0.0, double animationAngle = 0.0);
        void removeAnimation();
        void setText(QString text);
        void setColoured(bool state);
        unsigned getWidth();
        static void drawString(std::weak_ptr<sf::RenderWindow> destWindow, std::shared_ptr<BitmapFont> font, QString text,
            int x, int y, FontAlign align = FONT_ALIGN_LEFT);

    private:
        unsigned updateWidth();
        QString stringText;
        enum FontAlign align;
        std::shared_ptr<BitmapFont> textFont;
        bool isAnimated;
        bool isColoured;
        double varianceX;
        double varianceY;
        double phase;
        double animationSpeed;
        double angleOffset;
        char spacing;
        unsigned width;
        static unsigned getStaticWidth(QString text, std::shared_ptr<BitmapFont> font);

        static const sf::Color colouredFontColours[7];
};
