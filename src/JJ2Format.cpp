#include "JJ2Format.h"
#include <QString>
#include <QFile>
#include <QByteArray>
#include <QDataStream>

QByteArray JJ2Format::fromLittleEndian(QByteArray le) {
    QDataStream a(le);
    QByteArray res;
    a.setByteOrder(QDataStream::BigEndian);
    qint8 byte;
    while (!a.atEnd()) {
        a >> byte;
        res.prepend(byte);
    }
    return res;
}

QString JJ2Format::convertJ2B(const QString& original) {
    QFile outfile(original);
    QFile infile(QString("RunTimeConverted/converted.it"));
    if (!outfile.exists()) {
        return "";
    }
    if (!outfile.open(QIODevice::ReadOnly)) {
        return "";
    }

    // http://mods.jazz2online.com/j2b.txt
    // J2B header structure
    //     4 bytes, magic string "MUSE"
    //     4 bytes, magic number (little endian):
    //             0xDEADBABE (AMFF)
    //         or
    //             0xDEADBEAF (AM)
    //         By selective testing, it appears that J2B files from TSF (and most probably CC) use AM
    //         while the ones from the original version use AMFF.
    //     4 bytes, filesize
    //     4 bytes, CRC
    //     4 bytes, compressed data size
    //     4 bytes, uncompressed data size
    // qUncompress wants the last size so don't remove that, and we need the magic number to know
    // whether the data is AM or AMFF; skip CRC and sizes and assume the file is not corrupt
    if (outfile.read(4).toInt() != 0x4553554D) {
        outfile.close();
        return "";
    }

    int format = outfile.read(4).toInt();
    outfile.seek(8);

    QByteArray size_bytes = fromLittleEndian(outfile.read(4));    
    QByteArray data = qUncompress(size_bytes + outfile.readAll());
    
    outfile.close();
    if (!infile.open(QIODevice::WriteOnly)) {
        return "";
    }

    // rest is todo
    if (format == 0xAFBEADDE) {
        
    } else if (format == 0xBEBAADDE) {

    } else {
        // unknown format
    }

    // handle somehow the situation if the file already exists
    return "RunTimeConverted/converted.it";
}
