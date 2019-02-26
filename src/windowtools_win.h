#ifndef WINDOW_TOOLS_WIN_H
#define WINDOW_TOOLS_WIN_H

#include "windowtools.h"

#include <windows.h>
#include <QTimer>

/**
 * Windows implementation of the window tools.
 */
class WindowTools_Win : public WindowTools {
Q_OBJECT

public:
    WindowTools_Win();
    
    ~WindowTools_Win() override;
    
    /**
     * Look up and remembers Thunderbird window handle.
     *
     * @return true if found, false if not found.
     */
    bool lookup() override;
    
    /**
     * Shows / activates the window.
     *
     * @return true on success.
     */
    bool show() override;
    
    /**
     * Hides / closes the window (without closing the process).
     *
     * @return true on success.
     */
    bool hide() override;
    
    /**
     * @return Whether window is hidden or not.
     */
    bool isHidden() override;
    
    //
    /**
     * Asks Thunderbird to close.
     *
     * @return true if the request was made successfully.
     */
    bool closeWindow() override;
    
    /**
     * @return true if the Thunderbird window is valid (hidden or shown)
     */
    bool isValid() override;

private slots:
    /**
     * Timer callback to check if the Thunderbird window is minimized.
     */
    void timerWindowState();

private:
    /**
     * Ensure that the Thunderbid window is still valid or refresh it otherwise.
     *
     * @return true, if we have a valid Thunderbird window.
     */
    bool checkWindow();
    
    /**
     * Cool-down for the hide timer to not hide the window when we un-hide it.
     */
    int hideCoolDown;
    
    /**
     * The handle to the Thunderbird window.
     */
    HWND thunderbirdWindow;
    
    /**
     * Timer for the minimized window check.
     */
    QTimer windowStateTimer;
};

#endif // WINDOW_TOOLS_WIN_H
