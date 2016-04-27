#include "ShaderSource.h"
#include <QDir>

QMap<QString, std::shared_ptr<sf::Shader>> ShaderSource::shaders = QMap<QString, std::shared_ptr<sf::Shader>>();

bool ShaderSource::systemSupportsShaders = false;

const QString ShaderSource::dummyShaderName = "DummyShader";

bool ShaderSource::initialize() {
    systemSupportsShaders = sf::Shader::isAvailable();
    if (systemSupportsShaders) {
        loadShader("ColorizeShader");
        loadShader("LightingShader");
        loadDummyShader();
    }

    return systemSupportsShaders;
}

void ShaderSource::teardown() {
    shaders.clear();
}

std::shared_ptr<sf::Shader> ShaderSource::getShader(const QString& name) {
    if (systemSupportsShaders) {
        return shaders.value(name, shaders.value(dummyShaderName));
    }
    return shaders.value(dummyShaderName);
}

bool ShaderSource::loadShader(const QString& name) {
    if (name == dummyShaderName) {
        // Overwriting the dummy shader is not allowed
        return false;
    }

    auto shader = std::make_shared<sf::Shader>();
    if (shader->loadFromFile(QDir::current().absoluteFilePath("Shaders/" + name + ".glsl").toStdString(),
        sf::Shader::Fragment)) {
        shader->setParameter("texture", sf::Shader::CurrentTexture);
        shaders.insert(name, shader);
        return true;
    }
    return false;
}

bool ShaderSource::loadDummyShader() {
    auto shader = std::make_shared<sf::Shader>();
    if (shader->loadFromMemory("uniform sampler2D texture; void main() { gl_FragColor = texture2D(texture, gl_TexCoord[0].xy); }",
        sf::Shader::Fragment)) {
        shader->setParameter("texture", sf::Shader::CurrentTexture);
        shaders.insert(dummyShaderName, shader);
        return true;
    }
    return false;
}
