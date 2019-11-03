#ifndef WINDOW_TOOLS_WIN_H
#define WINDOW_TOOLS_WIN_H

#include "windowtools.h"

#include <windows.h>

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

private:
    /**
     * Ensure that the Thunderbird window is still valid or refresh it otherwise.
     *
     * @return true, if we have a valid Thunderbird window.
     */
    bool checkWindow();
    
    /**
     * Callback when the thunderbird window was minimized.
     *
     * @param eventHook The handle of this event hook.
     * @param event The minimize event.
     * @param window The window that was minimized.
     * @param idObject The object that was changed.
     * @param idChild Identifies if the cause of the event was a child of the object.
     * @param idEventThread The thread that belongs to the event.
     * @param eventTime The time of the event.
     */
    static void CALLBACK minimizeCallback(
            HWINEVENTHOOK eventHook, DWORD event, HWND window, LONG idObject,
            LONG idChild, DWORD idEventThread, DWORD eventTime);
    
    /**
     * The handle to the minimize hook.
     */
    HWINEVENTHOOK thunderbirdMinimizeHook;
    
    /**
     * The handle to the Thunderbird window.
     */
    HWND thunderbirdWindow;
};

#endif // WINDOW_TOOLS_WIN_H
