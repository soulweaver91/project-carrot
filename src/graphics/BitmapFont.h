#pragma once

#include <QString>
#include <QList>
#include <SFML/Graphics.hpp>

enum FontAlign {
    FONT_ALIGN_LEFT = 0,
    FONT_ALIGN_RIGHT = 1,
    FONT_ALIGN_CENTER = 2
};

class BitmapFont {
    public:
        BitmapFont(const QString& filename, unsigned width, unsigned height, unsigned rows, unsigned cols, unsigned first, unsigned last);
        sf::Sprite* getCharacterSprite(QChar code);
        unsigned getCharacterWidth(QChar code);
    private:
        short char_width[256];
        sf::Texture font_tex;
        QList< sf::Sprite* > char_map;
        short first_char;
};

class BitmapString {
    public:
        BitmapString(BitmapFont* font, const QString& init_str = "", FontAlign init_align = FONT_ALIGN_LEFT);
        void drawString(sf::RenderWindow* destWindow, int x, int y);
        void setAnimation(bool set_anim = false, double x_var = 0.0, double y_var = 0.0,
                          double a_speed = 0.0, double a_angle = 0.0);
        void removeAnimation();
        void setText(QString text);
        void setColoured(bool state);
        unsigned getWidth();
        static void drawString(sf::RenderWindow* destWindow, BitmapFont* font, QString text, int x, int y, FontAlign align = FONT_ALIGN_LEFT);

    private:
        unsigned updateWidth();
        QString str_text;
        enum FontAlign align;
        BitmapFont* inner_font;
        bool animate;
        bool coloured;
        double xvariance;
        double yvariance;
        double phase;
        double anim_speed;
        double angle_offset;
        char spacing;
        unsigned width;
        sf::Color Colour[7];
        static unsigned getStaticWidth(QString text, BitmapFont* font);
};
