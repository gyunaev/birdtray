#ifndef WINDOWTOOLS_X11_H
#define WINDOWTOOLS_X11_H

#include "windowtools.h"

#include <QTimer>
#include <QRegularExpression>
#include <QList>

#include <X11/Xatom.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xutil.h>

class WindowTools_X11 : public WindowTools
{
    Q_OBJECT

    public:
        WindowTools_X11();
        ~WindowTools_X11();

        // Looks up and remembers Thunderbird window handle. Returns true if found,
        // false if not found.
        virtual bool    lookup();

        // Shows/activates the window
        virtual bool    show();

        // Hides/closes the window (without closing the process)
        virtual bool    hide();

        // Whether window is hidden or not
        virtual bool    isHidden();

        // Closes the application via WM_CLOSE or similar
        virtual bool    closeWindow();

        // Return true if Thunderbird window is valid (hidden or shown)
        virtual bool    isValid();

    private slots:
        void    doHide();
        void    timerWindowState();

    private:
        Display *x11_display();
        Window  x11_appRootWindow();

        // Makes sure our window ID is still valid, or reinitializes it
        bool    checkWindow();

        // Our Window ID
        Window      mWinId;

        // Size
        XSizeHints  mSizeHint;

        // State counter
        int         mHiddenStateCounter;

        // State check timer
        QTimer      mWindowStateTimer;
};

#endif // WINDOWTOOLS_X11_H
