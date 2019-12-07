#include <QApplication>
#include "birdtrayapp.h"


int main(int argc, char *argv[]) {
    BirdtrayApp app(argc, argv);

    if ( app.isInstanceRunning() )
        return 0;

    return QApplication::exec();
}
