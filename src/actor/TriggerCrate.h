#pragma once

#include <memory>

#include "../CarrotQt5.h"
#include "SolidObject.h"

class TriggerCrate : public SolidObject {
    public:
        TriggerCrate(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0, int trigger_id = 0);
        ~TriggerCrate();
        bool perish() override;
    private:
        int trigger_id;
};
