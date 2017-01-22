#pragma once

#include <QString>
#include <functional>
#include "../struct/PCEvent.h"
#include "ActorAPI.h"

typedef std::shared_ptr<ActorAPI> apiHandle;
typedef std::function<std::shared_ptr<CommonActor>(bool, int, int, const quint16[8])> SpawnerFunction;

struct EventRepresentation {
    PCEvent eventID;
    QString identifier;
    SpawnerFunction spawner;
};

class EventSpawner {
public:
    EventSpawner();
    ~EventSpawner();

    bool initializeSpawnableList();
    void setApi(apiHandle newApi);
    void registerSpawnable(PCEvent type, const QString& resourceName, SpawnerFunction spawner);
    std::shared_ptr<CommonActor> spawnEvent(bool fromEventMap, PCEvent type, int x, int y, const quint16 params[8]) const;
    QString getEventResourceName(PCEvent type) const;

    template<typename T, typename... P>
    void registerTrivialSpawnable(PCEvent type, const QString& resourceName, P... params);

private:
    std::weak_ptr<ActorAPI> api;
    QMap<PCEvent, EventRepresentation> spawnableEvents;

    template<typename T, typename... P>
    std::shared_ptr<CommonActor> createCommonActorEvent(bool fromEventMap, int x, int y, P... params);
};
