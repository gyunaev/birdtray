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
    static WindowTools * tools;

    if ( !tools )
    {
#if defined (Q_OS_WIN)
    tools = 0;
#elif defined (Q_OS_MAC)
    tools = 0;
#else
    tools = new WindowTools_X11();
#endif
    }

    return tools;
}
