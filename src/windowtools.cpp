#include <QtCore>

#include "windowtools.h"
#include "windowtools_x11.h"

WindowTools::WindowTools()
    : QObject()
{
}

WindowTools::~WindowTools()
{
}


WindowTools *WindowTools::create()
{
#if defined (Q_OS_WIN)
    return 0;
#elif defined (Q_OS_MAC)
    return 0;
#else
    return new WindowTools_X11();
#endif
}
