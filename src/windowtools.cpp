#include <QtCore>

#include "windowtools.h"
#if defined (Q_OS_WIN)
#  include "windowtools_win.h"
#elif defined (Q_OS_MAC)
// TODO
#else
#  include "windowtools_x11.h"
#endif


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
    tools = new WindowTools_Win();
#elif defined (Q_OS_MAC)
    tools = 0;
#else
    tools = new WindowTools_X11();
#endif
    }

    return tools;
}
