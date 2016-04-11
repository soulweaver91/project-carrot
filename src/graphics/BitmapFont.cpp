#include "BitmapFont.h"
#include <QFile>
#include "../struct/Constants.h"

BitmapFont::BitmapFont(const QString& filename, unsigned width, unsigned height, unsigned rows, 
    unsigned cols, unsigned first, unsigned last) {
    if (!(fontTexture.loadFromFile(filename.toUtf8().data()))) {
        throw 0;
    }

    char widthFromFileTable[256];
    std::fill_n(widthFromFileTable, 256, static_cast<char>(width % 256));

    QFile widthFile(filename + ".config");
    if (!(widthFile.open(QIODevice::ReadOnly))) {
        // oops, something went wrong
    } else {
        widthFile.read(widthFromFileTable,256);
    }
    widthFile.close();

    int charX = 0;
    int charY = 0;
    int charCode = 0;
    firstCharCode = first;
    unsigned i = 0;
    for (; i < first; ++i) {
        charWidth[i] = 0;
    }
    for (; i < last; ++i, ++charCode) {
        charWidth[i] = widthFromFileTable[charCode];
        auto new_spr = std::make_shared<sf::Sprite>(fontTexture, sf::IntRect(charX, charY, widthFromFileTable[charCode], height));
        characterMap.push_back(new_spr);
        charX += width;
        if (charCode % cols == (cols - 1)) {
            charY += height;
            charX = 0;
        }
        if ((charCode > last) || (i >= 255)) {
            break;
        }
    }
    for (; i < 256; ++i) {
        charWidth[i] = 0;
    }
}

std::shared_ptr<sf::Sprite> BitmapFont::getCharacterSprite(QChar code) {
    short num = code.toLatin1() - firstCharCode;
    if (num < 0 || num >= characterMap.size()) {
        return nullptr;
    } else {
        return characterMap.at(num);
    }
}

unsigned BitmapFont::getCharacterWidth(QChar code) {
    short num = code.toLatin1() - firstCharCode;
    if (num < 0 || num >= characterMap.size()) {
        return 0;
    } else {
        return characterMap.at(num)->getTextureRect().width;
    }
}

const sf::Color BitmapString::colouredFontColours[7] = {
    sf::Color(192, 192, 255),
    sf::Color(255, 192, 192),
    sf::Color(255, 255, 192),
    sf::Color( 24, 255, 115),
    sf::Color(255, 192, 255),
    sf::Color(180, 180, 240),
    sf::Color(255, 255, 255)
};

BitmapString::BitmapString(std::shared_ptr<BitmapFont> font, const QString& initString, FontAlign initAlign) :
    align(initAlign), isAnimated(false), varianceX(0.0), varianceY(0.0), phase(0.0), animationSpeed(0.0),
    angleOffset(0.0), stringText(initString), isColoured(false), spacing(-1) {
    textFont = font;
    updateWidth();
}

void BitmapString::drawString(std::weak_ptr<sf::RenderWindow> destWindow, int x, int y) {
    auto canvas = destWindow.lock();
    if (canvas == nullptr) {
        return;
    }

    int currentX = x;
    if (align == FONT_ALIGN_CENTER) {
        currentX -= getWidth() / 2;
    } else if (align == FONT_ALIGN_RIGHT) {
        currentX -= getWidth();
    }
    double differenceX = 0.0;
    double differenceY = 0.0;
    for (int i = 0; i < stringText.length(); ++i) {
        if (isAnimated) {
            differenceX = cos(((phase + i) * angleOffset) * PI) * varianceX;
            differenceY = sin(((phase + i) * angleOffset) * PI) * varianceY;
        }
        auto sprite = textFont->getCharacterSprite(stringText.at(i));
        if (sprite != nullptr) {
            if (isColoured) {
                sprite->setColor(colouredFontColours[i % 7]);
            } else {
                sprite->setColor(sf::Color::White);
            }
            sprite->setPosition(currentX + differenceX, y + differenceY);
            canvas->draw(*(sprite));
            currentX += sprite->getTextureRect().width + spacing;
        }
    }
    if (isAnimated) {
        phase += animationSpeed;
    }
}

void BitmapString::setAnimation(bool setAnimated, double varianceX, double varianceY, double animationSpeed, double animationAngle) {
    phase = 0.0;
    isAnimated = setAnimated;
    angleOffset = animationAngle;
    this->varianceX = varianceX;
    this->varianceY = varianceY;
    this->animationSpeed = animationSpeed;
}
void BitmapString::removeAnimation() {
    setAnimation();
}

void BitmapString::setText(QString text) {
    stringText = text;
    updateWidth();
}

unsigned BitmapString::updateWidth() {
    unsigned sum = 0;
    for(unsigned i = 0; i < stringText.length(); ++i) {
        sum += textFont->getCharacterWidth(stringText.at(i));
    }
    width = sum;
    return sum;
}

unsigned BitmapString::getWidth() {
    return width;
}

void BitmapString::drawString(std::weak_ptr<sf::RenderWindow> destWindow, std::shared_ptr<BitmapFont> font,
    QString text, int x, int y, FontAlign align) {
    auto canvas = destWindow.lock();
    if (canvas == nullptr) {
        return;
    }

    int currentX = x;
    if (align == FONT_ALIGN_CENTER) {
        currentX -= getStaticWidth(text, font) / 2;
    } else if (align == FONT_ALIGN_RIGHT) {
        currentX -= getStaticWidth(text, font);
    }
    double differenceX = 0.0;
    double differenceY = 0.0;
    for (int i = 0; i < text.length(); ++i) {
        auto sprite = font->getCharacterSprite(text.at(i));
        if (sprite != nullptr) {
            sprite->setPosition(currentX + differenceX, y + differenceY);
            sprite->setColor(sf::Color::White);
            canvas->draw(*(sprite));
            currentX += sprite->getTextureRect().width - 1;
        }
    }
}

unsigned BitmapString::getStaticWidth(QString text, std::shared_ptr<BitmapFont> font) {
    unsigned sum = 0;
    for(unsigned i = 0; i < text.length(); ++i) {
        sum += font->getCharacterWidth(text.at(i));
    }
    return sum;
}

void BitmapString::setColoured(bool state) {
    isColoured = state;
}
