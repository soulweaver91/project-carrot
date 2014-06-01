/*************************************************************************
 ** PROJECT CARROT                                                      **
 *************************************************************************/

#include "CarrotQt5.h"
#include "QSFMLCanvas.h"

#include <QApplication>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <bass.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CarrotQt5 w;
    w.show();
    w.parseCommandLine();
    return a.exec();
}
