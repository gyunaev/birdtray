#include "windowtools_win.h"
#include <tlhelp32.h>
#include "settings.h"
#include "utils.h"

/**
 * Helper data structure for the findMainWindow function.
 */
struct _WindowFindData {
    DWORD processId;
    HWND windowHandle;
};

/**
 * Determine whether a window handle is the main window of the corresponding process.
 *
 * @param handle The window handle.
 * @return True if the handle is the main window handle.
 */
static bool isMainWindow(HWND handle) {
    return GetWindow(handle, GW_OWNER) == static_cast<HWND>(nullptr) && IsWindowVisible(handle);
}

/**
 * Check if the given window is the main window of the given process.
 *
 * @param handle The handle of the window.
 * @param parameter In/Out parameter, determining the id of the process and
 *                  providing a space to store the window handle.
 * @return TRUE if the window does not match the search criteria.
 */
static BOOL CALLBACK enumWindowsCallback(HWND handle, LPARAM parameter) {
    _WindowFindData &data = *reinterpret_cast<_WindowFindData*>(parameter);
    DWORD processId = 0;
    GetWindowThreadProcessId(handle, &processId);
    if (data.processId != processId || !isMainWindow(handle)) {
        return TRUE;
    }
    data.windowHandle = handle;
    return FALSE;
}

/**
 * Find the main window of the given process.
 *
 * @param process_id The id of the target process.
 * @return The main window of the process or nullptr.
 */
static HWND findMainWindow(DWORD process_id) {
    _WindowFindData data = {};
    data.processId = process_id;
    data.windowHandle = nullptr;
    EnumWindows(enumWindowsCallback, reinterpret_cast<LPARAM>(&data));
    return data.windowHandle;
}

/**
 * Get the process id of a process by the name of the executable.
 *
 * @param processName The name of the executable of the process.
 * @return The process id or 0.
 */
static DWORD getProcessId(LPCWCH processName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (_wcsicmp(entry.szExeFile, processName) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        }
    }
    CloseHandle(snapshot);
    return 0;
}


WindowTools_Win::WindowTools_Win() : WindowTools() {
    thunderbirdWindow = nullptr;
    hideCoolDown = 0;
    connect(&windowStateTimer, &QTimer::timeout, this, &WindowTools_Win::timerWindowState);
    windowStateTimer.setInterval(250);
    windowStateTimer.start();
}

WindowTools_Win::~WindowTools_Win() {
    windowStateTimer.stop();
    thunderbirdWindow = nullptr;
}

bool WindowTools_Win::lookup() {
    if (isValid()) {
        return thunderbirdWindow;
    }
    
    DWORD thunderbirdProcessId = getProcessId(L"thunderbird.exe");
    if (thunderbirdProcessId == 0) {
        return false;
    }
    
    thunderbirdWindow = findMainWindow(thunderbirdProcessId);
    return thunderbirdWindow != nullptr;
}

bool WindowTools_Win::show() {
    if (!checkWindow()) {
        return false;
    }
    hideCoolDown = 2;
    ShowWindow(this->thunderbirdWindow, SW_SHOW);
    if (IsIconic(this->thunderbirdWindow)) {
        ShowWindow(this->thunderbirdWindow, SW_RESTORE);
    }
    return SetForegroundWindow(this->thunderbirdWindow) == TRUE;
}

bool WindowTools_Win::hide() {
    if (!checkWindow()) {
        return false;
    }
    return ShowWindow(this->thunderbirdWindow, SW_HIDE) > 0;
}

bool WindowTools_Win::isHidden() {
    return isValid() && !IsWindowVisible(this->thunderbirdWindow);
}

bool WindowTools_Win::closeWindow() {
    if (!checkWindow()) {
        return false;
    }
    show();
    return SendMessage(this->thunderbirdWindow, WM_CLOSE, 0, 0) == 0;
}

bool WindowTools_Win::isValid() {
    return thunderbirdWindow != nullptr && IsWindow(thunderbirdWindow);
}

void WindowTools_Win::timerWindowState() {
    if (!pSettings->mHideWhenMinimized || hideCoolDown-- > 0 || !isValid()) {
        return;
    }
    hideCoolDown = 0;
    if (IsIconic(this->thunderbirdWindow) && IsWindowVisible(this->thunderbirdWindow)) {
        this->hide();
    }
}

bool WindowTools_Win::checkWindow() {
    return isValid() || lookup();
}
