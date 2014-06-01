#ifndef H_CP_TRIGCRATE
#define H_CP_TRIGCRATE

#include "CommonActor.h"
#include "SolidObject.h"
#include "CarrotQt5.h"
#include <SFML/Graphics.hpp>

class TriggerCrate : public SolidObject {
    public:
        TriggerCrate(CarrotQt5* root, double x = 0.0, double y = 0.0, int trigger_id = 0);
        ~TriggerCrate();
        bool perish() override;
    private:
        int trigger_id;
};

#endif
