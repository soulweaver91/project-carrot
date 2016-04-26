#include "ShaderSource.h"
#include <QDir>

QMap<QString, std::shared_ptr<sf::Shader>> ShaderSource::shaders = QMap<QString, std::shared_ptr<sf::Shader>>();

bool ShaderSource::systemSupportsShaders = false;

bool ShaderSource::initialize() {
    systemSupportsShaders = sf::Shader::isAvailable();
    if (systemSupportsShaders) {
        loadShader("ColorizeShader");
        loadShader("LightingShader");
    }

    return systemSupportsShaders;
}

void ShaderSource::teardown() {
    shaders.clear();
}

std::shared_ptr<sf::Shader> ShaderSource::getShader(const QString& name) {
    if (systemSupportsShaders) {
        return shaders.value(name);
    }
    return nullptr;
}

bool ShaderSource::loadShader(const QString& name) {
    auto shader = std::make_shared<sf::Shader>();
    if (shader->loadFromFile(QDir::current().absoluteFilePath("Shaders/" + name + ".glsl").toStdString(),
        sf::Shader::Fragment)) {
        shader->setParameter("texture", sf::Shader::CurrentTexture);
        shaders.insert(name, shader);
        return true;
    }
    return false;
}

