#pragma once

#include "CommonActor.h"
#include "SolidObject.h"
#include "CarrotQt5.h"
#include <SFML/Graphics.hpp>

class PushBox : public SolidObject {
    public:
        PushBox(CarrotQt5* root, double x = 0.0, double y = 0.0, int type = PC_PUSHABLE_ROCK);
        ~PushBox();
};
