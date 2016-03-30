/*************************************************************************
 ** PROJECT CARROT                                                      **
 *************************************************************************/

#include <memory>
#include "CarrotQt5.h"
#include "QSFMLCanvas.h"

#include <QApplication>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <bass.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::shared_ptr<CarrotQt5> w = std::make_shared<CarrotQt5>();
    w->show();
    w->parseCommandLine();
    return a.exec();
}
