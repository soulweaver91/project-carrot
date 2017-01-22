#pragma once
#include <QSet>
#include <QVector>
#include "../../struct/WeaponTypes.h"

class AmmoContainer {
protected:
    QVector<WeaponType> generateRandomAmmo(QSet<WeaponType> options) {
        int count = 5 + qrand() % 3;

        if (options.size() == 0) {
            return QVector<WeaponType>(count, WEAPON_BOUNCER);
        }

        QVector<WeaponType> types;
        for (int i = 0; i < count; ++i) {
            types << *(options.begin() + qrand() % options.size());
        }

        return types;
    }
};
