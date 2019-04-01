#ifndef PROCESS_HANDLE_WIN_H
#define PROCESS_HANDLE_WIN_H

#include <windows.h>
#include "processhandle.h"
#include <QThread>


/**
 * Process handle for the Windows platform.
 */
class ProcessHandle_Win: public ProcessHandle {
public:
    explicit ProcessHandle_Win(QString executablePath);
    ~ProcessHandle_Win() override;
    AttachResult attach() override;

private:
    
    /**
     * Thread that waits for a process to exit.
     */
    class ProcessWaiter: public QThread {
    friend ProcessHandle_Win;
    protected:
        void run() override;

    private:
        explicit ProcessWaiter(ProcessHandle_Win* processHandle):
                processHandle(processHandle), QThread() {}
        ProcessHandle_Win* processHandle;
    };
    
    /**
     * @return Weather the handle is attached to a process.
     */
    bool isAttached();
    
    /**
     * Get a handle to the running process for this handle,
     * or nullptr if no such process is running.
     *
     * @return A handle to the running process or nullptr.
     */
    HANDLE findProcess();
    
    /**
     * A handle to the process.
     */
    HANDLE processHandle = nullptr;
    
    /**
     * A thread waiting for the process to exit.
     */
    QThread* processWaiter = nullptr;
};


#endif // PROCESS_HANDLE_WIN_H
