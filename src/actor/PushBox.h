#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

#include "../CarrotQt5.h"
#include "SolidObject.h"

class PushBox : public SolidObject {
    public:
        PushBox(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0, int type = PC_PUSHABLE_ROCK);
        ~PushBox();
};
