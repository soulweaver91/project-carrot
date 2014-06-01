#ifndef H_CP_SOLIDOBJECT
#define H_CP_SOLIDOBJECT

#include "CommonActor.h"
#include "CarrotQt5.h"
#include <SFML/Graphics.hpp>

class SolidObject : public CommonActor {
    public:
        SolidObject(CarrotQt5* root, double x = 0.0, double y = 0.0, bool movable = true);
        ~SolidObject();
        void push(bool left);
        bool isOneWay();
        virtual bool perish() override;
    protected:
        bool movable;
        bool one_way;
};

#endif
