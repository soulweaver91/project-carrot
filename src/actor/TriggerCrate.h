#pragma once

#include <memory>

#include "SolidObject.h"

class CarrotQt5;

class TriggerCrate : public SolidObject {
    public:
        TriggerCrate(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0, int trigger_id = 0);
        ~TriggerCrate();
        bool perish() override;
    private:
        int trigger_id;
};
