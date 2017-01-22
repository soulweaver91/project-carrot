#pragma once

#include "SolidObject.h"
#include "../../struct/PCEvent.h"

class GenericContainer : public SolidObject {
public:
    GenericContainer(const ActorInstantiationDetails& initData);
    ~GenericContainer();
    bool perish() override;

protected:
    void insertActor(std::shared_ptr<CommonActor> actor);
    void empty();
    void generateContents(PCEvent type, uint count);

    QVector<std::shared_ptr<CommonActor>> contents;
};