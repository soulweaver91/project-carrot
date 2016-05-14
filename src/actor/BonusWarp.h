#pragma once

#include "CommonActor.h"
#include "../struct/CoordinatePair.h"

class BonusWarp : public CommonActor {
public:
    BonusWarp(std::shared_ptr<CarrotQt5> root, double x, double y, const quint16 params[8]);
    ~BonusWarp();
    const CoordinatePair getWarpTarget();
    void getParams(quint16 (&params)[8]);

private:
    quint16 eventParams[8];
};
