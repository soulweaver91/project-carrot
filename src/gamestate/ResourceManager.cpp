#include "ResourceManager.h"
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QVector>

ResourceManager::ResourceManager() {
    soundSystem = std::make_shared<SoundSystem>();
    graphicsCache = std::make_shared<GraphicsCache>();
}

ResourceManager::~ResourceManager() {

}

std::shared_ptr<GraphicResource> ResourceManager::requestTexture(const QString& filename) {
    return graphicsCache->request(filename);
}

std::shared_ptr<SoundSystem> ResourceManager::getSoundSystem() {
    return soundSystem;
}

std::shared_ptr<GraphicsCache> ResourceManager::getGraphicsCache() {
    return graphicsCache;
}

std::shared_ptr<ResourceSet> ResourceManager::loadActorTypeResources(const QString& actorType) {
    if (loadedActorResources.contains(actorType)) {
        return loadedActorResources.value(actorType);
    }

    std::shared_ptr<ResourceSet> set = std::make_shared<ResourceSet>();

    QFile manifestFile(QDir::current().absoluteFilePath("Data/Manifests/" + actorType + ".json"));

    if (!manifestFile.exists() || !manifestFile.open(QIODevice::ReadOnly)) {
        return set;
    }

    QJsonDocument manifest;
    QJsonParseError err;

    manifest = QJsonDocument::fromJson(manifestFile.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        return set;
    }

    if (!manifest.isObject()
        || !manifest.object().value("graphics").isObject()
        || !manifest.object().value("sounds").isObject()) {
        return set;
    }

    const QJsonObject graphics = manifest.object().value("graphics").toObject();
    const QJsonObject sounds = manifest.object().value("sounds").toObject();
    if (manifest.object().value("children").isArray()) {
        const QJsonArray children = manifest.object().value("children").toArray();
        foreach(QJsonValue child, children) {
            if (child.isString()) {
                loadActorTypeResources(child.toString());
            }
        }
    }

    foreach(const QString& graphicId, graphics.keys()) {
        if (!graphics.value(graphicId).isObject()) {
            continue;
        }
        QJsonObject graphic = graphics.value(graphicId).toObject();

        if (!graphic.contains("filename") || !graphic.value("filename").isString()) {
            continue;
        }

        auto resource = graphicsCache->request(graphic.value("filename").toString());
        if (resource == nullptr) {
            continue;
        }

        // We want a local version to which overrides can be made.
        auto localResource = std::make_shared<GraphicResource>(*resource);
        
        if (graphic.contains("states") && graphic.value("states").isArray()) {
            foreach(QJsonValue value, graphic.value("states").toArray()) {
                if (value.isString()) {
                    localResource->state.insert(value.toString().toUInt());
                }
            }
        }

        if (graphic.contains("fps") && graphic.value("fps").isDouble()) {
            int value = graphic.value("fps").toInt();
            if (value > 0) {
                localResource->frameDuration = 1000 / value;
            } else {
                localResource->frameDuration = 0;
            }
        }

        if (graphic.contains("frameOffset") && graphic.value("frameOffset").isDouble()) {
            int value = graphic.value("frameOffset").toInt();
            if (value > 0) {
                localResource->frameOffset = std::min((uint)value, localResource->frameCount - 1);
            }
        }

        if (graphic.contains("frameCount") && graphic.value("frameCount").isDouble()) {
            int value = graphic.value("frameCount").toInt();
            if (value > 0) {
                localResource->frameCount = std::min((uint)value, localResource->frameCount - localResource->frameOffset);
            }
        }
        localResource->frameCount = std::min(localResource->frameCount, resource->frameCount - localResource->frameOffset);

        set->graphics.insert(graphicId, localResource);
    }
    foreach(const QString& soundId, sounds.keys()) {
        if (!sounds.value(soundId).isObject()) {
            continue;
        }
        QJsonObject sound = sounds.value(soundId).toObject();

        if (!sound.contains("filename")) {
            continue;
        }

        QVector<QString> names;
        if (sound.value("filename").isArray()) {
            foreach(auto item, sound.value("filename").toArray()) {
                if (item.isString()) {
                    names << item.toString();
                }
            }
        } else if (sound.value("filename").isString()) {
            names << sound.value("filename").toString();
        }

        foreach(QString name, names) {
            auto handle = soundSystem->addSFX(soundId, name);
            if (handle == 0) {
                continue;
            }

            SoundResource res;
            res.sound = handle;
            set->sounds.insert(soundId, res);
        }
    }
    
    loadedActorResources.insert(actorType, set);
    return set;
}

