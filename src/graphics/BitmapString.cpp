#include "BitmapString.h"

#include <cmath>
#include <algorithm>
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
    align(initAlign), textFont(font), isAnimated(false), isColoured(false),
    varianceX(0.0), varianceY(0.0), phase(0.0), animationSpeed(0.0), angleOffset(0.0), spacing(font->getDefaultSpacing()) {
    stringText = parseString(initString);
    updateWidth();
}

void BitmapString::drawString(sf::RenderTarget* canvas, int x, int y) {
    double differenceX = 0.0;
    double differenceY = 0.0;
    uint lineHeight = textFont->getLineHeight();
    int colorIdx = 0;
    int i = 0;
    int lineIdx = 0;
    for (auto& line : stringText) {
        int currentX = x;
        if (align == FONT_ALIGN_CENTER) {
            currentX -= width[lineIdx] / 2;
        } else if (align == FONT_ALIGN_RIGHT) {
            currentX -= width[lineIdx];
        }

        for (auto& character : line) {

            if (isAnimated) {
                differenceX = std::cos(((phase + i) * angleOffset) * PI) * varianceX;
                differenceY = std::sin(((phase + i) * angleOffset) * PI) * varianceY;
            }
            auto sprite = textFont->getCharacterSprite(character);
            if (sprite != nullptr) {
                sf::RenderStates state;
                auto shader = ShaderSource::getShader("ColorizeShader").get();
                if (isColoured) {
                    auto color = colouredFontColours[colorIdx % 7];
                    shader->setParameter("color", color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
                    state.shader = shader;
                }
                sprite->setPosition(currentX + differenceX, y + differenceY + lineIdx * lineHeight);
                canvas->draw(*(sprite), state);
                currentX += sprite->getTextureRect().width + spacing;
            }

            if (character != 0x20) {
                colorIdx++;
            }

            i++;
        }
        lineIdx++;
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
    stringText = parseString(text);
    updateWidth();
}

unsigned BitmapString::updateWidth() {
    width.clear();
    for (auto& line : stringText) {
        width.append(getStaticLineWidth(line, textFont, spacing));
    }
    return getWidth();
}

unsigned BitmapString::getWidth() {
    return *std::max_element(width.begin(), width.end());
}

void BitmapString::drawString(sf::RenderTarget* canvas, std::shared_ptr<BitmapFont> font,
    QString text, int x, int y, FontAlign align) {
    uint lineHeight = font->getLineHeight();
    uint lineIdx = 0;

    for (auto& line : parseString(text)) {
        uint width = getStaticLineWidth(line, font, font->getDefaultSpacing());
        int currentX = x;
        if (align == FONT_ALIGN_CENTER) {
            currentX -= width / 2;
        } else if (align == FONT_ALIGN_RIGHT) {
            currentX -= width;
        }
        double differenceX = 0.0;
        double differenceY = 0.0;
        for (int i = 0; i < line.length(); ++i) {
            auto sprite = font->getCharacterSprite(line.at(i));
            if (sprite != nullptr) {
                sprite->setPosition(currentX + differenceX, y + differenceY + lineIdx * lineHeight);
                sprite->setColor(sf::Color::White);
                canvas->draw(*(sprite));
                currentX += sprite->getTextureRect().width + font->getDefaultSpacing();
            }
        }
        lineIdx++;
    }
}

unsigned BitmapString::getStaticLineWidth(const QString& line, std::shared_ptr<BitmapFont> font, char spacing) {
    unsigned sum = 0;
    for (QChar ch : line) {
        sum += font->getCharacterWidth(ch);
    }
    return sum + (line.length() - 1) * spacing;
}

QStringList BitmapString::parseString(const QString& string) {
    return string.split('@');
}

void BitmapString::setColoured(bool state) {
    isColoured = state;
}
