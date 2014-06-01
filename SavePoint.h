#ifndef H_CP_SAVEPOINT
#define H_CP_SAVEPOINT

#include "CommonActor.h"
#include "CarrotQt5.h"

class SavePoint : public CommonActor {

    Q_OBJECT

public:
    SavePoint(CarrotQt5* root, double x = 0.0, double y = 0.0);
    ~SavePoint();
    void tickEvent();
    void activateSavePoint();

private:
    bool activated;
};

#endif
