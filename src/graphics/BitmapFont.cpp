#include "BitmapFont.h"
#include <QFile>

#define PI 3.1415926535

BitmapFont::BitmapFont(const QString& filename, unsigned width, unsigned height, unsigned rows, unsigned cols, unsigned first, unsigned last) {
    if (!(font_tex.loadFromFile(filename.toUtf8().data()))) {
        throw 0;
    }

    char w_table[256];
    std::fill_n(w_table, 256, static_cast< char >(width % 256));

    QFile widthfile(filename + ".config");
    if (!(widthfile.open(QIODevice::ReadOnly))) {
        // oops, something went wrong
    } else {
        widthfile.read(w_table,256);
    }
    widthfile.close();

    int curr_x = 0;
    int curr_y = 0;
    int curr_id = 0;
    first_char = first;
    unsigned i = 0;
    for (; i < first; ++i) {
        char_width[i] = 0;
    }
    for (; i < last; ++i, ++curr_id) {
        char_width[i] = w_table[curr_id];
        sf::Sprite* new_spr = new sf::Sprite(font_tex,sf::IntRect(curr_x,curr_y,w_table[curr_id],height));
        char_map.push_back(new_spr);
        curr_x += width;
        if (curr_id % cols == (cols - 1)) {
            curr_y += height;
            curr_x = 0;
        }
        if ((curr_id > last) || (i >= 255)) {
            break;
        }
    }
    for (; i < 256; ++i) {
        char_width[i] = 0;
    }
}

sf::Sprite* BitmapFont::getCharacterSprite(QChar code) {
    short num = code.toLatin1() - first_char;
    if (num < 0 || num >= char_map.size()) {
        return nullptr;
    } else {
        return char_map.at(num);
    }
}

unsigned BitmapFont::getCharacterWidth(QChar code) {
    short num = code.toLatin1() - first_char;
    if (num < 0 || num >= char_map.size()) {
        return 0;
    } else {
        return char_map.at(num)->getTextureRect().width;
    }
}

BitmapString::BitmapString(BitmapFont* font, const QString& init_str, FontAlign init_align) :
    align(init_align), animate(false), xvariance(0.0), yvariance(0.0), phase(0.0), anim_speed(0.0),
    angle_offset(0.0), str_text(init_str), coloured(false), spacing(-1) {
    inner_font = font;
    updateWidth();

    // Initialize the inner colour list
    sf::Color a[7] = {
            sf::Color(192,192,255),
            sf::Color(255,192,192),
            sf::Color(255,255,192),
            sf::Color(24,255,115),
            sf::Color(255,192,255),
            sf::Color(180,180,240),
            sf::Color(255,255,255)
        };
    for (int i = 0; i < 7; ++i) {
        Colour[i] = a[i];
    }
}

void BitmapString::drawString(std::weak_ptr<sf::RenderWindow> destWindow, int x, int y) {
    auto canvas = destWindow.lock();
    if (canvas == nullptr) {
        return;
    }

    int curr_x = x;
    if (align == FONT_ALIGN_CENTER) {
        curr_x -= getWidth() / 2;
    } else if (align == FONT_ALIGN_RIGHT) {
        curr_x -= getWidth();
    }
    double diff_x = 0.0;
    double diff_y = 0.0;
    for (int i = 0; i < str_text.length(); ++i) {
        if (animate) {
            diff_x = cos(((phase + i)*angle_offset) * PI) * xvariance;
            diff_y = sin(((phase + i)*angle_offset) * PI) * yvariance;
        }
        sf::Sprite* spr = inner_font->getCharacterSprite(str_text.at(i));
        if (spr != nullptr) {
            if (coloured) {
                spr->setColor(Colour[i % 7]);
            } else {
                spr->setColor(sf::Color::White);
            }
            spr->setPosition(curr_x + diff_x, y + diff_y);
            canvas->draw(*(spr));
            curr_x += spr->getTextureRect().width + spacing;
        }
    }
    if (animate) {
        phase += anim_speed;
    }
}

void BitmapString::setAnimation(bool set_anim, double x_var, double y_var, double a_speed, double a_angle) {
    animate = set_anim;
    xvariance = x_var;
    yvariance = y_var;
    phase = 0.0;
    anim_speed = a_speed;
    angle_offset = a_angle;
}
void BitmapString::removeAnimation() {
    setAnimation();
}

void BitmapString::setText(QString text) {
    str_text = text;
    updateWidth();
}

unsigned BitmapString::updateWidth() {
    unsigned sum = 0;
    for(unsigned i = 0; i < str_text.length(); ++i) {
        sum += inner_font->getCharacterWidth(str_text.at(i));
    }
    width = sum;
    return sum;
}

unsigned BitmapString::getWidth() {
    return width;
}

void BitmapString::drawString(std::weak_ptr<sf::RenderWindow> destWindow, BitmapFont* font, QString text, int x, int y, FontAlign align) {
    auto canvas = destWindow.lock();
    if (canvas == nullptr) {
        return;
    }

    int curr_x = x;
    if (align == FONT_ALIGN_CENTER) {
        curr_x -= getStaticWidth(text,font) / 2;
    } else if (align == FONT_ALIGN_RIGHT) {
        curr_x -= getStaticWidth(text,font);
    }
    double diff_x = 0.0;
    double diff_y = 0.0;
    for (int i = 0; i < text.length(); ++i) {
        sf::Sprite* spr = font->getCharacterSprite(text.at(i));
        if (spr != nullptr) {
            spr->setPosition(curr_x + diff_x, y + diff_y);
            spr->setColor(sf::Color::White);
            canvas->draw(*(spr));
            curr_x += spr->getTextureRect().width + 1;
        }
    }
}

unsigned BitmapString::getStaticWidth(QString text, BitmapFont* font) {
    unsigned sum = 0;
    for(unsigned i = 0; i < text.length(); ++i) {
        sum += font->getCharacterWidth(text.at(i));
    }
    return sum;
}

void BitmapString::setColoured(bool state) {
    coloured = state;
}
