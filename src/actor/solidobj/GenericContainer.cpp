#include "GenericContainer.h"
#include "../../gamestate/ActorAPI.h"

GenericContainer::GenericContainer(const ActorInstantiationDetails& initData) : SolidObject(initData) {
}

GenericContainer::~GenericContainer() {
}

bool GenericContainer::perish() {
    if (health == 0) {
        for (auto actor : contents) {
            api->addActor(actor);
            actor->moveInstantly({ posX + qrand() % 160 / 10.0 - 8.0, posY + qrand() % 160 / 10.0 - 8.0 }, true, true);
        }
        empty();
    }
    return CommonActor::perish();
}

void GenericContainer::insertActor(std::shared_ptr<CommonActor> actor) {
    contents << actor;
}

void GenericContainer::empty() {
    contents.clear();
}

void GenericContainer::generateContents(PCEvent type, uint count) {
    for (uint i = 0; i < count; ++i) {
        quint16 fakeParams[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        auto actor = api->createActor(type, posX, posY, fakeParams, true);
        if (actor != nullptr) {
            insertActor(actor);
        }
    }

}
