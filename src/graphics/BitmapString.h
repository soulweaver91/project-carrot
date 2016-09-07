#pragma once

#include <memory>
#include <QString>
#include <QStringList>
#include <QVector>
#include <SFML/Graphics.hpp>

enum FontAlign {
    FONT_ALIGN_LEFT = 0,
    FONT_ALIGN_RIGHT = 1,
    FONT_ALIGN_CENTER = 2
};

enum BitmapFontSize {
    LARGE,
    NORMAL,
    SMALL
};

class BitmapFont;

class BitmapString {
public:
    BitmapString(std::shared_ptr<BitmapFont> font, const QString& initString = "", FontAlign initAlign = FONT_ALIGN_LEFT);
    void drawString(sf::RenderTarget* canvas, int x, int y);
    void setAnimation(bool setAnimated = false, double varianceX = 0.0, double varianceY = 0.0,
        double animationSpeed = 0.0, double animationAngle = 0.0);
    void removeAnimation();
    void setText(QString text);
    void setColoured(bool state);
    unsigned getWidth();
    static void drawString(sf::RenderTarget* canvas, std::shared_ptr<BitmapFont> font, QString text,
        int x, int y, FontAlign align = FONT_ALIGN_LEFT);

private:
    unsigned updateWidth();
    QStringList stringText;
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
    QVector<unsigned> width;
    static unsigned getStaticLineWidth(const QString& text, std::shared_ptr<BitmapFont> font, char spacing);
    static QStringList parseString(const QString& string);

    static const sf::Vector3i colouredFontColours[7];
};
