#include "ShaderSource.h"
#include <QDir>
#include <cassert>

ShaderSource::ShaderSource() {
    systemSupportsShaders = sf::Shader::isAvailable();
    if (systemSupportsShaders) {
        loadShader("ColorizeShader");
    }
}

ShaderSource::~ShaderSource() {

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

