#include "BitmapString.h"

#include <cmath>
#include "../struct/Constants.h"
#include "../graphics/ShaderSource.h"
#include "BitmapFont.h"

const sf::Vector3i BitmapString::colouredFontColours[7] = {
    sf::Vector3i(-16, 0, 1536),
    sf::Vector3i(256, -96, 64),
    sf::Vector3i(256, 64, -64),
    sf::Vector3i(-160, 32, -96),
    sf::Vector3i(64, -48, 192),
    sf::Vector3i(-32, -32, 64),
    sf::Vector3i(192, 192, 192)
};

BitmapString::BitmapString(std::shared_ptr<BitmapFont> font, const QString& initString, FontAlign initAlign) :
    stringText(initString), align(initAlign), textFont(font), isAnimated(false), isColoured(false),
    varianceX(0.0), varianceY(0.0), phase(0.0), animationSpeed(0.0), angleOffset(0.0), spacing(-1) {
    updateWidth();
}

void BitmapString::drawString(sf::RenderTarget* canvas, int x, int y) {
    int currentX = x;
    if (align == FONT_ALIGN_CENTER) {
        currentX -= getWidth() / 2;
    } else if (align == FONT_ALIGN_RIGHT) {
        currentX -= getWidth();
    }
    double differenceX = 0.0;
    double differenceY = 0.0;
    int c = 0;
    for (int i = 0; i < stringText.length(); ++i) {
        if (isAnimated) {
            differenceX = std::cos(((phase + i) * angleOffset) * PI) * varianceX;
            differenceY = std::sin(((phase + i) * angleOffset) * PI) * varianceY;
        }
        auto sprite = textFont->getCharacterSprite(stringText.at(i));
        if (sprite != nullptr) {
            sf::RenderStates state;
            auto shader = ShaderSource::getShader("ColorizeShader").get();
            if (isColoured) {
                auto color = colouredFontColours[c % 7];
                shader->setParameter("color", color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
                state.shader = shader;
            }
            sprite->setPosition(currentX + differenceX, y + differenceY);
            canvas->draw(*(sprite), state);
            currentX += sprite->getTextureRect().width + spacing;
        }

        if (stringText.at(i) != 0x20) {
            c++;
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
    for (QChar ch : stringText) {
        sum += textFont->getCharacterWidth(ch);
    }
    width = sum + (stringText.length() - 1) * spacing;
    return sum;
}

unsigned BitmapString::getWidth() {
    return width;
}

void BitmapString::drawString(sf::RenderTarget* canvas, std::shared_ptr<BitmapFont> font,
    QString text, int x, int y, FontAlign align) {
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
    for (QChar ch : text) {
        sum += font->getCharacterWidth(ch);
    }

    return sum - (text.length() - 1);
}

void BitmapString::setColoured(bool state) {
    isColoured = state;
}
