#include <QTimer>

#include "windowtools_x11.h"


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

/*
 * The Grand Window Analyzer. Checks if window w has a expected pid of epid
 * or a expected name of ename.
 */
static bool analyzeWindow(Display *display, Window w, const QString &ename)
{
    XClassHint ch;

    // no plans to analyze windows without a name
    char *window_name = NULL;
    if (!XFetchName(display, w, &window_name)) {
        return false;
    }
    if (window_name) {
        XFree(window_name);
    } else {
        return false;
    }

    bool this_is_our_man = false;
    // lets try the program name
    if (XGetClassHint(display, w, &ch)) {
        if (QString(ch.res_name).endsWith(ename)) {
            this_is_our_man = true;
        } else if (QString(ch.res_class).endsWith(ename)) {
            this_is_our_man = true;
        } else {
            // sheer desperation
            char *wm_name = NULL;
            XFetchName(display, w, &wm_name);
            if (wm_name && QString(wm_name).endsWith(ename)) {
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
    Window w = None;
    Window root;
    Window parent;
    Window *child;
    unsigned int num_child;

    if (XQueryTree(display, window, &root, &parent, &child, &num_child) != 0) {
        for (unsigned int i = 0; i < num_child; i++) {
            if (analyzeWindow(display, child[i], ename)) {
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

            w = findWindow(display, child[i], checkNormality, ename);
            if (w != None) {
                break;
            }
        }
    }
    return w;
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
    return w;
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
}

WindowTools_X11::~WindowTools_X11()
{
}

bool WindowTools_X11::lookup()
{
    mWinId = findWindow( QX11Info::display(), QX11Info::appRootWindow(), true, pSettings->mThunderbirdWindowMatch );

    qDebug("Window ID found: %lu", mWinId );

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
        qDebug("Warning: trying to hide already hidden window");

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

void WindowTools_X11::doHide()
{
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

    mHiddenStateCounter++;

    if ( mHiddenStateCounter == 2 )
        qDebug("Window removed from taskbar");
}

bool WindowTools_X11::checkWindow()
{
    if ( mWinId == None || !isValidWindowId( QX11Info::display(), mWinId ) )
        return lookup();

    return true;
}
