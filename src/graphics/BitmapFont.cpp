#include "BitmapFont.h"

#include <QFile>

BitmapFont::BitmapFont(const QString& filename, unsigned width, unsigned height,
    unsigned cols, unsigned first, unsigned last, int defaultSpacing) : defaultSpacing(defaultSpacing) {
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

    uint charCode = 0;
    firstCharCode = first;
    unsigned i = 0;
    for (; i < first; ++i) {
        charWidth[i] = 0;
    }
    for (; i < last; ++i, ++charCode) {
        charWidth[i] = widthFromFileTable[charCode];
        auto new_spr = std::make_shared<sf::Sprite>(fontTexture, sf::IntRect(
            ((i - first) % cols) * width,
            ((i - first) / cols) * height,
            widthFromFileTable[charCode],
            height));
        characterMap.push_back(new_spr);
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

int BitmapFont::getDefaultSpacing() {
    return defaultSpacing;
}
