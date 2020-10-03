#include <QTimer>

#include "birdtrayapp.h"
#include "windowtools_x11.h"
#include "utils.h"
#include "log.h"

/*
 * This code is mostly taken from xlibutil.cpp KDocker project, licensed under GPLv2 or higher.
 * The original code is copyrighted as following:
 *  Copyright (C) 2009, 2012, 2015 John Schember <john@nachtimwald.com>
 *  Copyright (C) 2004 Girish Ramakrishnan All Rights Reserved.
 *
 * THIS IS MODIFIED VERSION by George Yunaev, the modifications mostly excluded unused code,
 * and adapted it for KWin on Plasma 5.
 */

/*
 * Assert validity of the window id. Get window attributes for the heck of it
 * and see if the request went through.
 */
static bool isValidWindowId(Display *display, Window w) {
    XWindowAttributes attrib;
    return (XGetWindowAttributes(display, w, &attrib) != 0);
}

/*
 * Checks if this window is a normal window (i.e)
 * - Has a WM_STATE
 * - Not modal window
 * - Not a purely transient window (with no window type set)
 * - Not a special window (desktop/menu/util) as indicated in the window type
 */
static bool isNormalWindow(Display *display, Window w) {
    Atom type;
    int format;
    unsigned long left;
    Atom *data = NULL;
    unsigned long nitems;
    Window transient_for = None;

    static Atom wmState      = XInternAtom(display, "WM_STATE", false);
    static Atom windowState  = XInternAtom(display, "_NET_WM_STATE", false);
    static Atom modalWindow  = XInternAtom(display, "_NET_WM_STATE_MODAL", false);
    static Atom windowType   = XInternAtom(display, "_NET_WM_WINDOW_TYPE", false);
    static Atom normalWindow = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", false);
    static Atom dialogWindow = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", false);

    int ret = XGetWindowProperty(display, w, wmState, 0, 10, false, AnyPropertyType, &type, &format, &nitems, &left, (unsigned char **) & data);

    if (ret != Success || data == NULL) {
        if (data != NULL)
            XFree(data);
        return false;
    }
    if (data) {
        XFree(data);
    }

    ret = XGetWindowProperty(display, w, windowState, 0, 10, false, AnyPropertyType, &type, &format, &nitems, &left, (unsigned char **) & data);
    if (ret == Success) {
        unsigned int i;
        for (i = 0; i < nitems; i++) {
            if (data[i] == modalWindow) {
                break;
            }
        }
        XFree(data);
        if (i < nitems) {
            return false;
        }
    }

    XGetTransientForHint(display, w, &transient_for);

    ret = XGetWindowProperty(display, w, windowType, 0, 10, false, AnyPropertyType, &type, &format, &nitems, &left, (unsigned char **) & data);

    if ((ret == Success) && data) {
        unsigned int i;
        for (i = 0; i < nitems; i++) {
            if (data[i] != normalWindow && data[i] != dialogWindow) {
                break;
            }
        }
        XFree(data);
        return (i == nitems);
    } else {
        return (transient_for == None);
    }
}

/*
Window XLibUtil::pidToWid(Display *display, Window window, bool checkNormality, pid_t epid, QList<Window> dockedWindows) {
    Window w = None;
    Window root;
    Window parent;
    Window *child;
    unsigned int num_child;

    if (XQueryTree(display, window, &root, &parent, &child, &num_child) != 0) {
        for (unsigned int i = 0; i < num_child; i++) {
            if (epid == pid(display, child[i])) {
                if (!dockedWindows.contains(child[i])) {
                    if (checkNormality) {
                        if (isNormalWindow(display, child[i])) {
                            return child[i];
                        }
                    } else {
                        return child[i];
                    }
                }
            }
            w = pidToWid(display, child[i], checkNormality, epid);
            if (w != None) {
                break;
            }
        }
    }

    return w;
}
*/

static QString getWindowName( Display *display, Window w )
{
    // Credits: https://stackoverflow.com/questions/8925377/why-is-xgetwindowproperty-returning-null
    Atom nameAtom = XInternAtom( display, "_NET_WM_NAME", false );
    Atom utf8Atom = XInternAtom( display, "UTF8_STRING", false );
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char *data = 0;
    QString out;

    if ( Success == XGetWindowProperty( display, w, nameAtom, 0, 65536, false, utf8Atom, &type, &format, &nitems, &after, &data))
    {
        out = QString::fromUtf8( (const char*) data );
        XFree(data);
    }

    return out;
}

/*
 * The Grand Window Analyzer. Checks if window w has a expected pid of epid
 * or a expected name of ename.
 */
static bool analyzeWindow(Display *display, Window w, const QString &ename )
{
    XClassHint ch;

    bool this_is_our_man = false;

    // Find the window name


    // lets try the program name
    if (XGetClassHint(display, w, &ch))
    {
        if (QString(ch.res_name).endsWith(ename)) {
            this_is_our_man = true;
        } else if (QString(ch.res_class).endsWith(ename)) {
            this_is_our_man = true;
        } else {
            // sheer desperation
            if ( getWindowName( display, w ).endsWith(ename) ) {
                this_is_our_man = true;
            }
        }

        if (ch.res_class) {
            XFree(ch.res_class);
        }
        if (ch.res_name) {
            XFree(ch.res_name);
        }
    }

    // it's probably a good idea to check (obsolete) WM_COMMAND here
    return this_is_our_man;
}

/*
 * Given a starting window look though all children and try to find a window
 * that matches the ename.
 */
static Window findWindow(Display *display, Window window, bool checkNormality, const QString &ename, QList<Window> dockedWindows = QList<Window>() )
{
    Window targetWindow = None;
    Window root;
    Window parent;
    Window *children;
    unsigned int num_child;

    if (XQueryTree(display, window, &root, &parent, &children, &num_child) != 0) {
        for (unsigned int i = 0; i < num_child; i++) {
            if (analyzeWindow(display, children[i], ename) && !dockedWindows.contains(children[i])
                && (!checkNormality || isNormalWindow(display, children[i]))) {
                targetWindow = children[i];
                break;
            }
            targetWindow = findWindow(display, children[i], checkNormality, ename);
            if (targetWindow != None) {
                break;
            }
        }
        XFree(children);
    }
    return targetWindow;
}

/*
 * Sends ClientMessage to a window.
 */
static void sendMessage(Display* display, Window to, Window w, const char *type, int format, long mask, void* data, int size) {
    XEvent ev;
    memset(&ev, 0, sizeof (ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = w;
    ev.xclient.message_type = XInternAtom(display, type, true);
    ev.xclient.format = format;
    memcpy((char *) & ev.xclient.data, (const char *) data, size);
    XSendEvent(display, to, false, mask, &ev);
    XSync(display, false);
}

/*
 * Returns the id of the currently active window.
 */
static Window activeWindow(Display * display) {
    Atom active_window_atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", true);
    Atom type = None;
    int format;
    unsigned long nitems, after;
    unsigned char *data = NULL;
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    int r = XGetWindowProperty(display, root, active_window_atom, 0, 1, false, AnyPropertyType, &type, &format, &nitems, &after, &data);

    Window w = None;
    if ((r == Success) && data && (*reinterpret_cast<Window *> (data) != None)) {
        w = *(Window *) data;
    } else {
        int revert;
        XGetInputFocus(display, &w, &revert);
    }
    if (r == Success) {
        XFree(data);
    }
    return w;
}

/*
 GY:  Unfortunately this doesn't work at least on KWin - the state changes, but close button is not disabled.
static bool disableCloseButton( Display * display, Window w )
{
    // see https://specifications.freedesktop.org/wm-spec/wm-spec-1.3.html#idm140130317577760
    static Atom windowState  = XInternAtom( display, "_NET_WM_ALLOWED_ACTIONS", false );
    static Atom atomClose = XInternAtom( display, "_NET_WM_ACTION_CLOSE", false );
    Atom type = None;
    int format;
    unsigned long nitems, after;
    Atom *data = NULL;
    QVector<Atom> newdata;

    int r = XGetWindowProperty(display, w, windowState, 0, 10, false, AnyPropertyType, &type, &format, &nitems, &after, (unsigned char**) &data);

    if ( r != Success)
        return false;

    for (unsigned int i = 0; i < nitems; i++)
        if ( data[i] != atomClose )
            newdata.push_back( data[i] );

    XFree(data);

    XChangeProperty( display, w, windowState, type, format, PropModeReplace, (unsigned char *) newdata.data(), newdata.size() );
    XSync(display, False);
    return true;
}
*/

static bool checkWindowState( Display * display, Window w, const char * state )
{
    static Atom windowState  = XInternAtom( display, "_NET_WM_STATE", false );
    static Atom atomstate = XInternAtom( display, state, false );
    Atom type = None;
    int format;
    unsigned long nitems, after;
    Atom *data = NULL;

    int r = XGetWindowProperty(display, w, windowState, 0, 10, false, AnyPropertyType, &type, &format, &nitems, &after, (unsigned char**) &data);

    if (r == Success)
    {
        unsigned int i;

        for (i = 0; i < nitems; i++)
            if ( data[i] == atomstate )
                break;

        XFree(data);

        if (i < nitems)
            return true;
    }

    return false;
}

#if 0
/*
 * Have events associated with mask for the window set in the X11 Event loop
 * to the application.
 */
static void subscribe(Display *display, Window w, long mask) {
    Window root = RootWindow(display, DefaultScreen(display));
    XWindowAttributes attr;

    XGetWindowAttributes(display, w == None ? root : w, &attr);

    XSelectInput(display, w == None ? root : w, attr.your_event_mask | mask);
    XSync(display, false);
}

static void unSubscribe(Display *display, Window w) {
    XSelectInput(display, w, NoEventMask);
    XSync(display, false);
}

/*
 * Sets data to the value of the requested window property.
 */
static bool getCardinalProperty(Display *display, Window w, Atom prop, long *data) {
    Atom type;
    int format;
    unsigned long nitems, bytes;
    unsigned char *d = NULL;

    if (XGetWindowProperty(display, w, prop, 0, 1, false, XA_CARDINAL, &type, &format, &nitems, &bytes, &d) == Success && d) {
        if (data) {
            *data = *reinterpret_cast<long *> (d);
        }
        XFree(d);
        return true;
    }
    return false;
}
#endif


WindowTools_X11::WindowTools_X11()
    : WindowTools()
{  
    mWinId = None;
    mHiddenStateCounter = 0;

    connect( &mWindowStateTimer, &QTimer::timeout, this, &WindowTools_X11::timerWindowState );
    mWindowStateTimer.setInterval( 250 );
    mWindowStateTimer.start();
}

WindowTools_X11::~WindowTools_X11()
{
}

bool WindowTools_X11::lookup()
{
    if ( isValid() )
        return mWinId;

    mWinId = findWindow(QX11Info::display(), QX11Info::appRootWindow(), true,
            BirdtrayApp::get()->getSettings()->mThunderbirdWindowMatch);

    Log::debug("Window ID found: %lX", mWinId );

    return mWinId != None;
}

bool WindowTools_X11::show()
{
    if ( !checkWindow() )
        return false;

    Display *display = QX11Info::display();
    Window root = QX11Info::appRootWindow();

    // We are still minimizing
    if ( mHiddenStateCounter == 1 )
        return false;

    if ( mHiddenStateCounter == 2 )
    {
        XMapWindow( display, mWinId );
        mSizeHint.flags = USPosition;
        XSetWMNormalHints(display, mWinId, &mSizeHint );
    }

    XMapRaised( display, mWinId );
    XFlush( display );

    // Make it the active window
    // 1 == request sent from application. 2 == from pager.
    // We use 2 because KWin doesn't always give the window focus with 1.
    long l_active[2] = {2, CurrentTime};
    sendMessage( display, root, mWinId, "_NET_ACTIVE_WINDOW", 32, SubstructureNotifyMask | SubstructureRedirectMask, l_active, sizeof (l_active) );
    XSetInputFocus(display, mWinId, RevertToParent, CurrentTime);

    mHiddenStateCounter = 0;
    return true;
}

bool WindowTools_X11::hide()
{
    if ( !checkWindow() )
        return false;

    if ( mHiddenStateCounter != 0 )
    {
        Log::debug("Warning: trying to hide already hidden window (counter %d), ignored", mHiddenStateCounter );
        return false;
    }

    // Get screen number
    Display *display = QX11Info::display();
    long dummy;

    XGetWMNormalHints( display, mWinId, &mSizeHint, &dummy );

    // We call doHide() twice - at first call kWin only minimizes it,
    // and only the second call actually hides the window from the taskbar.
    QTimer::singleShot( 0, this, &WindowTools_X11::doHide );
    QTimer::singleShot( 0, this, &WindowTools_X11::doHide );
    return true;
}

bool WindowTools_X11::isHidden()
{
    return mHiddenStateCounter == 2 && mWinId != activeWindow( QX11Info::display() );
}

bool WindowTools_X11::closeWindow()
{
    if ( !checkWindow() )
        return false;

    show();

    // send _NET_CLOSE_WINDOW
    long l[5] = {0, 0, 0, 0, 0};
    sendMessage( QX11Info::display(), QX11Info::appRootWindow(), mWinId, "_NET_CLOSE_WINDOW", 32, SubstructureNotifyMask | SubstructureRedirectMask, l, sizeof (l));
    return true;
}

bool WindowTools_X11::isValid()
{
    return mWinId != None && isValidWindowId( QX11Info::display(), mWinId );
}

void WindowTools_X11::doHide()
{
    // This function may end up being called more than two times because isHidden() not only checks the counter,
    // but also checks the active window. Depending on window manager, the counter may get to 2 much faster than
    // window manager removes the window from an active window. This would result in multiple calls to doHide().
    if ( mHiddenStateCounter == 2 )
    {
        Log::debug("Window already should be removed from taskbar");
        return;
    }

    Display *display = QX11Info::display();
    long screen = DefaultScreen(display);

    /*
     * A simple call to XWithdrawWindow wont do. Here is what we do:
     * 1. Iconify. This will make the application hide all its other windows. For
     *    example, xmms would take off the playlist and equalizer window.
     * 2. Withdraw the window to remove it from the taskbar.
     */
    XIconifyWindow(display, mWinId, screen ); // good for effects too
    XSync(display, False);
    XWithdrawWindow(display, mWinId, screen );

    // Increase the counter but do not exceed 2
    mHiddenStateCounter++;

    if ( mHiddenStateCounter == 2 )
        Log::debug("Window removed from taskbar");
}

void WindowTools_X11::timerWindowState()
{
    if (mWinId == None || !BirdtrayApp::get()->getSettings()->mHideWhenMinimized) {
        return;
    }

    // _NET_WM_STATE_HIDDEN is set for minimized windows, so if we see it, this means it was minimized by the user
    if ( checkWindowState( QX11Info::display(), mWinId, "_NET_WM_STATE_HIDDEN" ) && mHiddenStateCounter == 0 )
    {
        mHiddenStateCounter = 1;
        QTimer::singleShot( 0, this, &WindowTools_X11::doHide );
    }
}

bool WindowTools_X11::checkWindow()
{
    if ( mWinId == None || !isValidWindowId( QX11Info::display(), mWinId ) )
        return lookup();

    return true;
}
