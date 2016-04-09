#pragma once

#include <QString>
#include <QFile>
#include <QByteArray>
#include <QDataStream>

class JJ2Format {
public:
    static QString convertJ2B(const QString& original);
    static QString convertJ2T(const QString& original);
    static QString convertJ2L(const QString& original);

private:
    // no instances of JJ2Format allowed
    JJ2Format();
    JJ2Format(const JJ2Format& original);
    ~JJ2Format();

    static QByteArray fromLittleEndian(QByteArray le_int);
};
