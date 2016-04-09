#pragma once

#include <memory>
#include <QMap>
#include <SFML/Graphics.hpp>
#include "../struct/Resources.h"

class GraphicsCache {
public:
    GraphicsCache();
    ~GraphicsCache();
    std::shared_ptr<GraphicResource> request(const QString& filename);
    bool flush();

private:
    bool loadGraphics(const QString& filename);
    QMap<QString, std::shared_ptr<GraphicResource>> cache;
    static const GraphicResource defaultResource;
};
