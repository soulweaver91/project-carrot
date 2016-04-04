#pragma once

#include <memory>
#include <QMultiMap>
#include <QMap>
#include <QString>
#include "../sound/SoundSystem.h"
#include "../graphics/GraphicsCache.h"
#include "../struct/AnimState.h"
#include "../struct/Resources.h"

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();
    std::shared_ptr<GraphicResource> requestTexture(const QString& filename);
    std::shared_ptr<SoundSystem> getSoundSystem();
    std::shared_ptr<GraphicsCache> getGraphicsCache();
    std::shared_ptr<ResourceSet> loadActorTypeResources(const QString& actorType);
private:
    
    std::shared_ptr<SoundSystem> soundSystem;
    std::shared_ptr<GraphicsCache> graphicsCache;
    QMap<QString, std::shared_ptr<ResourceSet>> loadedActorResources;
};
