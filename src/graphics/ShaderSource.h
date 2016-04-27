#pragma once

#include <memory>
#include <QString>
#include <QMap>
#include <SFML/Graphics.hpp>

class ShaderSource {
public:
    ShaderSource() = delete;
    ~ShaderSource() = delete;
    static bool initialize();
    static void teardown();
    static std::shared_ptr<sf::Shader> getShader(const QString& name);

private:
    static bool loadShader(const QString& name);
    static bool loadDummyShader();
    static QMap<QString, std::shared_ptr<sf::Shader>> shaders;
    static bool systemSupportsShaders;
    const static QString dummyShaderName;
};