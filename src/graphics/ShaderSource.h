#pragma once

#include <memory>
#include <QString>
#include <QMap>
#include <SFML/Graphics.hpp>

class ShaderSource {
public:
    ShaderSource();
    ~ShaderSource();
    std::shared_ptr<sf::Shader> getShader(const QString& name);

private:
    bool loadShader(const QString& name);
    QMap<QString, std::shared_ptr<sf::Shader>> shaders;
    bool systemSupportsShaders;
};